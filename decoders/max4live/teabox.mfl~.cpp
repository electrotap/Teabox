/* 
 *	teabox.mfl~
 *	External object targetted at Max for Live
 *	
 *	Brings audio in using PortAudio so that we can get Teabox sensor data
 *	Copyright © 2009 by Timothy Place
 *	74 Objects LLC
 *
 *	Licensed under the terms of the GNU LGPL version 2.1
 */

#include "ext.h"					// Max Header
#include "z_dsp.h"					// MSP Header
#include "ext_strings.h"			// String Functions
#include "commonsyms.h"				// Common symbols used by the Max 4.5 API
#include "ext_obex.h"				// Max Object Extensions (attributes) Header
#include "ext_atomic.h"
#include "ext_critical.h"
#include "ext_systhread.h"
#include "portaudio.h"
#ifdef MAC_VERSION
#include "pa_mac_core.h"
#endif


// Data Structure for this object
typedef struct _teabox_mfl {
    t_pxobject			obj;
	long				iovs;	
	long				vectorSize;			///< framesPerBuffer
	long				sampleRate;
	long				attrDevice;
	const PaDeviceInfo*	inputDeviceInfo;
#ifdef MAC_VERSION
	AudioDeviceID		macDeviceID;
#endif
	bool				isRunning;
    PaStream*			stream;
	float*				buffer;
	int					index;
	int					record_index;
	long				attrChannel;		///< channel no. on the device to which the Teabox is connected
	char				counter;			///< used for keeping track of the current sensor number
	float				data[9];			///< one container for the data from each sensor
	float				hw_version;			///< version number (major.minor) of the connected Teabox
	float				last_value;			///< used for error correction in the perform method
	void*				outlet;				///< outlet for status and version messages
} t_teabox_mfl;


// Prototypes
void		teabox_mfl_quittask(void);
void*		teabox_mfl_new(t_symbol *msg, short argc, t_atom *argv);					// New Object Creation Method
void		teabox_mfl_free(t_teabox_mfl *x);
void		teabox_mfl_assist(t_teabox_mfl *x, void *b, long msg, long arg, char *dst);	// Assistance Method
void		teabox_mfl_getversion(t_teabox_mfl *x);
void		teabox_mfl_getstatus(t_teabox_mfl *x);
void		teabox_mfl_getdevices(t_teabox_mfl *x);
void		teabox_mfl_initStream(t_teabox_mfl *x);
void		teabox_mfl_start(t_teabox_mfl *x);
void		teabox_mfl_stop(t_teabox_mfl *x);
int			teabox_mfl_stream_callback( const void*						input,
										void*							output,
										unsigned long					frameCount,
										const PaStreamCallbackTimeInfo*	timeInfo,
										PaStreamCallbackFlags			statusFlags,
										void*							userData );
#ifdef MAC_VERSION
OSStatus	teabox_mfl_deviceListenerProc(  AudioDeviceID				inDevice,
											UInt32					inLine,
											Boolean					isInput,
											AudioDevicePropertyID	inPropertyID,
											void*					inClientData);
#endif
t_int*		teabox_mfl_perform(t_int *w);
void		teabox_mfl_dsp(t_teabox_mfl *x, t_signal **sp, short *count);
t_max_err	teabox_mfl_setdevice(t_teabox_mfl *x, void *attr, long argc, t_atom *argv);


// Globals
static t_class *s_teabox_mfl_class;


/************************************************************************************/
// Main() Function

int main(void)
{
	t_class*	c;
	PaError		paErr;
#ifdef EXPIRES
	t_datetime	max_datetime;
	
	systime_datetime(&max_datetime);
	if ((max_datetime.year >= 2009) && (max_datetime.month >= 5)) {
		error("teabox.mfl~: test version of object expired");
		return 1;
	}
	else {
		post("teabox.mfl~: test version of object expires May 1, 2009");
	}
#endif // EXPIRES
	
	common_symbols_init();
	
	c = class_new("teabox.mfl~",(method)teabox_mfl_new, (method)teabox_mfl_free, sizeof(t_teabox_mfl), (method)0L, A_GIMME, 0);
	
  	class_addmethod(c, (method)teabox_mfl_start,		"start",		0);
 	class_addmethod(c, (method)teabox_mfl_stop,			"stop",			0);
 	class_addmethod(c, (method)teabox_mfl_getdevices,	"getdevices",	0);
	class_addmethod(c, (method)teabox_mfl_getdevices,	"loadbang",		0);			// automatically populate menu on load
	class_addmethod(c, (method)teabox_mfl_getversion,	"getversion",	0);
	class_addmethod(c, (method)teabox_mfl_getstatus,	"getstatus",	0);
	class_addmethod(c, (method)teabox_mfl_dsp,			"dsp",			A_CANT, 0);		
	class_addmethod(c, (method)teabox_mfl_assist,		"assist",		A_CANT, 0); 
    class_addmethod(c, (method)object_obex_dumpout,		"dumpout", 		A_CANT, 0);  
		
	CLASS_ATTR_LONG(c,			"device",		0,	t_teabox_mfl,	attrDevice);
	CLASS_ATTR_ACCESSORS(c,		"device",		NULL,	teabox_mfl_setdevice);
	
	CLASS_ATTR_LONG(c,			"channel",		0,	t_teabox_mfl,	attrChannel);
	CLASS_ATTR_FILTER_MIN(c,	"channel",		1);
	
	class_dspinit(c);
	class_register(CLASS_BOX, c);
	s_teabox_mfl_class = c;
	
	paErr = Pa_Initialize();
	if (paErr != paNoError)
		error("PortAudio error: %s", Pa_GetErrorText(paErr));
	
	quittask_install((method)teabox_mfl_quittask, NULL);
	
	return 0;
}


void teabox_mfl_quittask(void)
{
	PaError err = Pa_Terminate();
	if (err != paNoError)
		cpost("PortAudio error: %s\n", Pa_GetErrorText( err ) );
}


/************************************************************************************/
// Object Creation Method

void* teabox_mfl_new(t_symbol *msg, short argc, t_atom *argv)
{
    t_teabox_mfl*	x;
	short			i;
	
    x = (t_teabox_mfl*)object_alloc(s_teabox_mfl_class);
    if (x) {
		x->sampleRate = sys_getsr();
		x->vectorSize = sys_getblksize();
		x->iovs = sys_getmaxblksize();				// see comment in the dsp method
		x->attrChannel = 1;
		x->counter = 0;								// init member values
		x->hw_version = 0;

		attr_args_process(x,argc,argv);

		x->outlet = outlet_new(x, 0L);				// create the status/version outlet
    	object_obex_store((void*)x, _sym_dumpout, (object*)x->outlet);
	    dsp_setup((t_pxobject*)x, 1);
		for (i=0; i<9; i++) {
			outlet_new((t_pxobject *)x, "signal");
			x->data[i] = 0;
		}
		x->obj.z_misc = Z_NO_INPLACE;
		
		// if the device attr had been set, then this would not be NULL
		if (!x->inputDeviceInfo)
			object_attr_setlong(x, gensym("device"), 0);
	}
	return x;
}


void teabox_mfl_free(t_teabox_mfl *x)
{
	PaError err;

	dsp_free((t_pxobject *)x);
	
#ifdef MAC_VERSION
	OSStatus status;
	
	if (x->macDeviceID) {		
		status = AudioDeviceRemovePropertyListener(x->macDeviceID, 0, true, kAudioDevicePropertyStreamConfiguration,	teabox_mfl_deviceListenerProc);
		status = AudioDeviceRemovePropertyListener(x->macDeviceID, 0, true, kAudioDevicePropertyStreams,				teabox_mfl_deviceListenerProc);
		status = AudioDeviceRemovePropertyListener(x->macDeviceID, 0, true, kAudioDevicePropertyNominalSampleRate,		teabox_mfl_deviceListenerProc);
		status = AudioDeviceRemovePropertyListener(x->macDeviceID, 0, true, kAudioDevicePropertyDataSource,				teabox_mfl_deviceListenerProc);
		x->macDeviceID = NULL;
	}
#endif
	
	if (x->stream) {
		if (x->isRunning)
			teabox_mfl_stop(x);
		
		err = Pa_CloseStream(x->stream);
		if(err != paNoError)
			object_error((t_object*)x, "PortAudio error freeing engine: %s", Pa_GetErrorText(err));		
	}

	delete[] x->buffer;
}


/************************************************************************************/
// Methods bound to input/inlets

// Method for Assistance Messages
void teabox_mfl_assist(t_teabox_mfl *x, void *b, long msg, long arg, char *dst)
{
	if (msg == 1)		// Inlets
		strcpy (dst, "control messages");
	else if (msg == 2) {	// Outlets
		switch (arg) {
			case 0: strcpy(dst, "(signal) Demultiplexed Sensor Signal 1"); break;
			case 1: strcpy(dst, "(signal) Demultiplexed Sensor Signal 2"); break;
			case 2: strcpy(dst, "(signal) Demultiplexed Sensor Signal 3"); break;
			case 3: strcpy(dst, "(signal) Demultiplexed Sensor Signal 4"); break;
			case 4: strcpy(dst, "(signal) Demultiplexed Sensor Signal 5"); break;
			case 5: strcpy(dst, "(signal) Demultiplexed Sensor Signal 6"); break;
			case 6: strcpy(dst, "(signal) Demultiplexed Sensor Signal 7"); break;
			case 7: strcpy(dst, "(signal) Demultiplexed Sensor Signal 8"); break;
			case 8: strcpy(dst, "(signal) Digital Sensors, Encoded as INT"); break;
			case 9: strcpy(dst, "(attributes) dumpout"); break;
		}
	}
}


void teabox_mfl_getversion(t_teabox_mfl *x)
{
	short 		version_major, version_minor;
	char		version_string[64];
	t_symbol*	version_symbol;
	t_atom		version_atom;
	
	version_major = (int)((x->hw_version * 4095.0) + 0.49) >> 8;
	version_minor = (int)((x->hw_version * 4095.0) + 0.49) & 255;
	
	//post("Teabox Firmware Version: %i.%i", version_major, version_minor);
	sprintf(version_string, "%i.%i", version_major, version_minor);
	version_symbol = gensym(version_string);
	atom_setsym(&version_atom, version_symbol);
	outlet_anything(x->outlet, gensym("version"), 1, &version_atom);
}


void teabox_mfl_getstatus(t_teabox_mfl *x)
{
	t_atom	status_atom;
	
	if (x->hw_version)
		atom_setlong(&status_atom, 1);
	else
		atom_setlong(&status_atom, 0);
	outlet_anything(x->outlet, gensym("status"), 1, &status_atom);
}


void teabox_mfl_getdevices(t_teabox_mfl *x)
{
	int		numDevices = Pa_GetDeviceCount();
	const   PaDeviceInfo *deviceInfo;
	t_atom* av = (t_atom*)sysmem_newptr(sizeof(t_atom) * numDevices);
	
	for (int i=0; i<numDevices; i++) {
        deviceInfo = Pa_GetDeviceInfo(i);
		atom_setsym(av+i, gensym((char*)deviceInfo->name));
    }
	
	object_obex_dumpout(x, gensym("devices"), numDevices, av);
	
	if(av)
		sysmem_freeptr(av);
}


void teabox_mfl_initStream(t_teabox_mfl *x)
{
	PaError				err;
	bool				shouldRun = x->isRunning;
	PaStreamParameters	inputParameters;
#ifdef MAC_VERSION
	OSStatus			status;
#endif
	
	if (x->isRunning)
		teabox_mfl_stop(x);
	
	if (x->stream) {
		Pa_CloseStream(x->stream);
		x->stream = NULL;
	}
#ifdef MAC_VERSION
	if (x->macDeviceID) {		
		status = AudioDeviceRemovePropertyListener(x->macDeviceID, 0, true, kAudioDevicePropertyStreamConfiguration,	teabox_mfl_deviceListenerProc);
		status = AudioDeviceRemovePropertyListener(x->macDeviceID, 0, true, kAudioDevicePropertyStreams,				teabox_mfl_deviceListenerProc);
		status = AudioDeviceRemovePropertyListener(x->macDeviceID, 0, true, kAudioDevicePropertyNominalSampleRate,		teabox_mfl_deviceListenerProc);
		status = AudioDeviceRemovePropertyListener(x->macDeviceID, 0, true, kAudioDevicePropertyDataSource,			teabox_mfl_deviceListenerProc);
		x->macDeviceID = NULL;
	}
#endif
	
	inputParameters.channelCount = Pa_GetDeviceInfo(x->attrDevice)->maxInputChannels;
	inputParameters.device = x->attrDevice; // 0 is the default
	inputParameters.hostApiSpecificStreamInfo = NULL;
	inputParameters.sampleFormat = paFloat32;
	inputParameters.suggestedLatency = Pa_GetDeviceInfo(x->attrDevice)->defaultHighInputLatency;
	
	if (x->sampleRate < 11025.0)
		x->sampleRate = 44100.0;
	
	err = Pa_OpenStream(&x->stream,
						&inputParameters,
						NULL,
						x->sampleRate,
						paFramesPerBufferUnspecified,
						paNoFlag,							//flags that can be used to define dither, clip settings and more
						teabox_mfl_stream_callback,			//your callback function
						x);

	if (err != paNoError )
		object_error((t_object*)x, "PortAudio error creating TTAudioEngine: %s", Pa_GetErrorText(err));
	
#ifdef MAC_VERSION
	x->macDeviceID = PaMacCore_GetStreamInputDevice(x->stream);
	
	status = AudioDeviceAddPropertyListener(x->macDeviceID, 0, true, kAudioDevicePropertyStreamConfiguration,	teabox_mfl_deviceListenerProc, x);
	status = AudioDeviceAddPropertyListener(x->macDeviceID, 0, true, kAudioDevicePropertyStreams,				teabox_mfl_deviceListenerProc, x);
	status = AudioDeviceAddPropertyListener(x->macDeviceID, 0, true, kAudioDevicePropertyNominalSampleRate,		teabox_mfl_deviceListenerProc, x);
	status = AudioDeviceAddPropertyListener(x->macDeviceID, 0, true, kAudioDevicePropertyDataSource,			teabox_mfl_deviceListenerProc, x);
#endif
	
	// Now that the stream is initialized, we need to setup our own buffers for reading and writing.
	delete[] x->buffer;
	x->buffer = new float[x->iovs];
	
	if (shouldRun)
		teabox_mfl_start(x);
}


void teabox_mfl_start(t_teabox_mfl *x)
{
	PaError err = paNoError;
	
	if (!x->isRunning) {
		if(!x->stream)
			teabox_mfl_initStream(x);
		
		err = Pa_StartStream(x->stream);
		if (err != paNoError) 
			object_error((t_object*)x, "PortAudio error starting engine: %s", Pa_GetErrorText(err));
		
		x->isRunning = true;
		
		// trying to start the recording just one vector ahead of the playback to ensure that it gets buffered.
		x->index = 0;
		x->record_index = x->vectorSize;
	again:
		if (x->record_index > x->iovs) {
			x->record_index = x->record_index = x->iovs;
			goto again;
		}
	}
}


void teabox_mfl_stop(t_teabox_mfl *x)
{
	if (x->isRunning) {
		PaError err = paNoError;
		
		if (x->stream) {
			err = Pa_StopStream(x->stream);
			if (err != paNoError) 
				object_error((t_object*)x, "PortAudio error stopping engine: %s", Pa_GetErrorText(err));
		}
		x->isRunning = false;
	}
}


int teabox_mfl_stream_callback(const void*						input,
								void*							output,
								unsigned long					frameCount,
								const PaStreamCallbackTimeInfo*	timeInfo,
								PaStreamCallbackFlags			statusFlags,
								void*							userData )
{
	t_teabox_mfl *x = (t_teabox_mfl*)userData;
	float *indata = (float*)input;
	
	if (statusFlags == 0) {
		indata += (x->attrChannel-1);						// move to the offset for the specified channel in the buffer
		for (unsigned int i=0; i<frameCount; i++) {
			x->buffer[x->record_index] = *indata;
			x->record_index++;
			if (x->record_index == x->iovs)
				x->record_index = 0;
			indata += x->inputDeviceInfo->maxInputChannels;	// we jump through the buffer, leaping over interleaved channels.
		}		
	}

	return paContinue;
}


#ifdef MAC_VERSION
// Listen for Device Properties -- if we don't stop PortAudio then we can crash
OSStatus teabox_mfl_deviceListenerProc(	AudioDeviceID			inDevice,
							 			UInt32					inLine,
							 			Boolean					isInput,
							 			AudioDevicePropertyID	inPropertyID,
							 			void*					inClientData)
{
	t_teabox_mfl*	x = (t_teabox_mfl*)inClientData;
    OSStatus		err = noErr;
	bool			running;
	
	cpost("teabox.mfl~ device notification ");
	
    switch (inPropertyID) {
		case kAudioDevicePropertyNominalSampleRate:
		case kAudioDevicePropertyStreams:
        case kAudioDevicePropertyStreamConfiguration:
      	case kAudioDevicePropertyDataSource:
			running = x->isRunning;
			teabox_mfl_stop(x);
			if (x->stream) {
				Pa_CloseStream(x->stream);
				x->stream = NULL;
			}
			if (running)
				defer_low(x, (method)teabox_mfl_start, NULL, 0, NULL);
            break;   
	}
	post("\n");	
 	return err;
}

#endif


// Perform (signal) Method
t_int *teabox_mfl_perform(t_int *w)
{
   	t_teabox_mfl*	x = (t_teabox_mfl *)(w[1]);
	t_float*		out1 = (t_float *)(w[2]);		// sensor outputs [1-8]...
	t_float*		out2 = (t_float *)(w[3]);
	t_float*		out3 = (t_float *)(w[4]);
	t_float*		out4 = (t_float *)(w[5]);
	t_float*		out5 = (t_float *)(w[6]);
	t_float*		out6 = (t_float *)(w[7]);
	t_float*		out7 = (t_float *)(w[8]);
	t_float*		out8 = (t_float *)(w[9]);
	t_float*		out9 = (t_float *)(w[10]);		// toggle output [9]
	int				n = (int)(w[11]);
	float			value;
	long			bitmask;
	int				i = x->index;
	float*			f = x->buffer;
	
	if (x->obj.z_disabled) 
		goto bye;

	// read from the buffer that is filled by port audio
	if (x->buffer && x->isRunning) {
		while (n--) {
			// INPUT SECTION
			value = f[i];
			
			if (value < 0.0 || x->counter > 9) {		// If the sample is the start flag...
				if(x->last_value < 0.0)					// Actually - if all 16 toggles on the Teabox digital inputs
					x->data[8] = x->last_value;			//	are high, it will look identical to the start flag - so
				//  so we compensate for that here.
				x->counter = 0;
			}
			else if (x->counter == 0) {					// if the sample is hardware version number...
				x->hw_version = value * 8.0;							
				x->counter++;
			}	
			else {
				x->data[x->counter - 1] = value * 8.0;	// Normalize the range
				x->counter++;
			}
			
			// POST-PROCESS TOGGLE INPUT BITMASK
			if (x->data[8] < 0) {
				bitmask = x->data[8] * 32768;			// 4096 = 32768 / 8 (we already multiplied by 8)
				bitmask ^= -32768;
				bitmask = 32768 + (bitmask);			// 2^3
			}
			else
				bitmask = x->data[8] * 4096;			// 4096 = 32768 / 8 (we already multiplied by 8)
			
			
			// OUTPUT SECTION - PIPE THE STORED DATA OUT THE OUTLETS
			*out1++ = x->data[0];
			*out2++ = x->data[1];
			*out3++ = x->data[2];
			*out4++ = x->data[3];
			*out5++ = x->data[4];
			*out6++ = x->data[5];
			*out7++ = x->data[6];
			*out8++ = x->data[7];
			*out9++ = (float)bitmask;					// Contains the 16-bits of digital inputs
			
			x->last_value = value;						// store the input value for the next time around
			
			i++;
			if (i == x->iovs)
				i = 0;
		}
	}
	
bye:
	x->index = i;
    return w+12;
}


// DSP Method
void teabox_mfl_dsp(t_teabox_mfl *x, t_signal **sp, short *count)
{
	teabox_mfl_stop(x);
	
	// we don't really use this as the iovs, it's really the buffer size -- which we want to have be plenty large
	x->iovs = sys_getmaxblksize();

	if (sp[0]->s_n != x->vectorSize) {
		x->vectorSize = sp[0]->s_n;
		teabox_mfl_initStream(x);
	}
	
	dsp_add(teabox_mfl_perform, 11, x,
			sp[1]->s_vec, sp[2]->s_vec, sp[3]->s_vec, sp[4]->s_vec, sp[5]->s_vec, sp[6]->s_vec, sp[7]->s_vec, sp[8]->s_vec, sp[9]->s_vec, 
			sp[0]->s_n);
    teabox_mfl_getstatus(x);	// automatically report the status when the dsp is turned on	
	
	teabox_mfl_start(x);
}


t_max_err teabox_mfl_setdevice(t_teabox_mfl *x, void *attr, long argc, t_atom *argv)
{
	if (argc) {
		x->attrDevice = atom_getlong(argv);
		x->inputDeviceInfo = Pa_GetDeviceInfo(x->attrDevice);
		teabox_mfl_initStream(x);
	}
	return MAX_ERR_NONE;
}

