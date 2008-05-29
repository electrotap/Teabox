/*
 * Teabox bit decoder object for Max/MSP
 * Written by Tim Place
 * Copyright © 2004 by Electrotap L.L.C.
 *
 
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 * 
 */



#ifdef PD
#include "m_pd.h"
#include <stdio.h>
#define SETSYM SETSYMBOL
#define SETLONG SETFLOAT
#else
#include "ext.h"				// Required for all Max External Objects
#include "ext_strings.h"		// Used for the assistance strings
#include "ext_obex.h"
#include "z_dsp.h"				// Required for all MSP externals
#endif

static t_class *teabox_bits_class;				// Required. Global pointing to this class


// Data Structure for this object
typedef struct _teabox_bits			
{
#ifdef PD
	t_object		obj;
	t_float			f;
#else
	t_pxobject 		obj;
	void			*obex;
#endif
} t_teabox_bits;


// Prototypes for methods: need a method for each incoming message type:
static t_int *teabox_bits_perform(t_int *w);									// An MSP Perform (signal) Method
#ifdef PD
static void teabox_bits_dsp(t_teabox_bits *x, t_signal **sp);			// DSP Method
#else
static void teabox_bits_dsp(t_teabox_bits *x, t_signal **sp, short *count);			// DSP Method
static void teabox_bits_assist(t_teabox_bits *x, void *b, long m, long a, char *s);	// Assistance Method
#endif
void *teabox_bits_new(t_symbol *s, long ac, t_atom *at);


/************************************************************************************/
// Main() Function

#ifdef PD
void teabox_bits_tilde_setup(void)
{
    teabox_bits_class = class_new(gensym("teabox_bits~"), (t_newmethod)teabox_bits_new, 0, 
		sizeof(t_teabox_bits), 0, A_GIMME, 0);

	CLASS_MAINSIGNALIN(teabox_bits_class, t_teabox_bits, f);
    class_addmethod(teabox_bits_class, (t_method)teabox_bits_dsp, gensym("dsp"), A_NULL);

    class_sethelpsymbol(teabox_bits_class, gensym("help-teabox.bits~.pd"));
	class_addcreator((t_newmethod)teabox_bits_new, gensym("teabox.bits~"), A_GIMME, 0);
}

#else	/* Max/MSP */
int main(void)				// main recieves a copy of the Max function macros table
{
	t_class	*c;
	
	c = class_new("teabox.bits~", (method)teabox_bits_new, (method)dsp_free, (short)sizeof(t_teabox_bits), 
		0L, A_GIMME, 0);
	
	common_symbols_init();

	class_obexoffset_set(c, calcoffset(t_teabox_bits, obex));
	    					
	class_addmethod(c, (method)teabox_bits_dsp,			"dsp", A_CANT, 0);		// Bind method "teabox_bits_dsp" to the DSP call from MSP
	class_addmethod(c, (method)teabox_bits_assist,		"assist", A_CANT,0);	// Bind method "teabox_bits_assist" to assistance calls

	class_dspinit(c);														// Setup object's class to work with MSP
	class_register(CLASS_BOX, c);
	teabox_bits_class = c;

	return 0;
}
#endif


/************************************************************************************/
// Object Creation Method

void *teabox_bits_new(t_symbol *s, long ac, t_atom *at)
{
	t_teabox_bits *x;
	short i;
	
#ifdef PD
	if ((x = (t_teabox_bits *)pd_new(teabox_bits_class)))
	{
		for(i=0; i<16; i++)
			outlet_new(&x->obj, gensym("signal"));	// Create new signal outlet
	}
#else
	if ((x = (t_teabox_bits *)object_alloc(teabox_bits_class)))
	{
		dsp_setup((t_pxobject *)x,1);

		for(i=0; i<16; i++)
			outlet_new((t_object *)x, "signal");	// Create new signal outlet
	}
#endif
	return (x);
}



/************************************************************************************/
// Methods bound to input/inlets

#ifndef PD
// Method for Assistance Messages
static void teabox_bits_assist(t_teabox_bits *x, void *b, long msg, long arg, char *dst)
{
	if(msg == 1)		// Inlets
		strcpy(dst, "(signal) bitmasked input");
	else if(msg == 2)	// Outlets
		strcpy(dst, "(signal) bit decoded output");
}
#endif


// Perform (signal) Method - delay is a constant (not a signal)
static t_int *teabox_bits_perform(t_int *w)
{
	t_float *out[16];
	short	i;
	long	bitmask;
	
	t_teabox_bits	*x = (t_teabox_bits *)(w[1]);		
	t_float			*in = (t_float *)(w[2]);	// input
	int 			n = (int)(w[3]);
	for(i=0; i<16; i++)
		out[i] = (t_float *)(w[i+4]);		// sensor outputs [1-8]...
	
#ifndef PD
	if (x->obj.z_disabled) goto out;
#endif
	
	while(n--){
		// INPUT SECTION
		bitmask = *in++;		

		// OUTPUT SECTION - PIPE THE STORED DATA OUT THE OUTLETS
		*out[0]++ = (bitmask & 1) > 0;
		*out[1]++ = (bitmask & 2) > 0;
		*out[2]++ = (bitmask & 4) > 0;
		*out[3]++ = (bitmask & 8) > 0;
		*out[4]++ = (bitmask & 16) > 0;
		*out[5]++ = (bitmask & 32) > 0;
		*out[6]++ = (bitmask & 64) > 0;
		*out[7]++ = (bitmask & 128) > 0;
		*out[8]++ = (bitmask & 256) > 0;
		*out[9]++ = (bitmask & 512) > 0;
		*out[10]++ = (bitmask & 1024) > 0;
		*out[11]++ = (bitmask & 2048) > 0;
		*out[12]++ = (bitmask & 4096) > 0;
		*out[13]++ = (bitmask & 8192) > 0;
		*out[14]++ = (bitmask & 16384) > 0;
		*out[15]++ = (bitmask & 32768) > 0;
	}
#ifndef PD
out:		
#endif
	return (w+20);
}


// DSP Method
#ifdef PD
static void teabox_bits_dsp(t_teabox_bits *x, t_signal **sp)
#else
static void teabox_bits_dsp(t_teabox_bits *x, t_signal **sp, short *count)
#endif
{
    dsp_add(teabox_bits_perform, 19, x, sp[0]->s_vec, sp[0]->s_n,
    	sp[1]->s_vec, sp[2]->s_vec, sp[3]->s_vec, sp[4]->s_vec, sp[5]->s_vec, sp[6]->s_vec, sp[7]->s_vec, sp[8]->s_vec, 
    	sp[9]->s_vec, sp[10]->s_vec, sp[11]->s_vec, sp[12]->s_vec, sp[13]->s_vec, sp[14]->s_vec, sp[15]->s_vec, sp[16]->s_vec);
}

