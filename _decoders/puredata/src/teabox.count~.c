/*
 * Teabox counter object for Max/MSP
 * Written by Tim Place
 * Copyright © 2005 by Electrotap L.L.C.
 *
 * Pd port Copyright © 2007 by Olaf Matthes, <olaf.matthes@gmx.de>.
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

static t_class *count_tilde_class;					// Required. Global pointing to this class


// Data Structure for this object
typedef struct _count_tilde			// Data Structure for this object
{
#ifdef PD
	t_object ob;
	t_float f;
#else
	t_pxobject ob;					// Required by MSP (must be first)
#endif
	long		value;				// stored count value
} t_count_tilde;


// Prototypes for methods: need a method for each incoming message type:
#ifdef PD
static void count_tilde_dsp(t_count_tilde *x, t_signal **sp);			// DSP Method
#else
static void count_tilde_dsp(t_count_tilde *x, t_signal **sp, short *count);			// DSP Method
#endif
static void count_tilde_assist(t_count_tilde *x, void *b, long m, long a, char *s);	// Assistance Method
static void *count_tilde_new(t_symbol *s, long argc, t_atom *argv);				// New Object Creation Method
static t_int *count_tilde_perform(t_int *w);
static void count_tilde_free(t_count_tilde *x);
static void count_tilde_activate(t_count_tilde *x, short toggle);


/************************************************************************************/
// Main() Function
#ifdef PD
void teabox_count_tilde_setup(void)
{
    count_tilde_class = class_new(gensym("teabox_count~"), (t_newmethod)count_tilde_new, 0, 
		sizeof(t_count_tilde), 0, A_GIMME, 0);

	CLASS_MAINSIGNALIN(count_tilde_class, t_count_tilde, f);
    class_addmethod(count_tilde_class, (t_method)count_tilde_dsp, gensym("dsp"), A_NULL);

    class_sethelpsymbol(count_tilde_class, gensym("help-teabox.count~.pd"));
	class_addcreator((t_newmethod)count_tilde_new, gensym("teabox.count~"), A_GIMME, 0);
}

#else	/* Max/MSP */
int main(void)	
{
	t_class	*c;
	
	c = class_new("teabox.count~", (method)count_tilde_new, (method)dsp_free, (short)sizeof(t_count_tilde), 
		0L, A_GIMME, 0);
	
	common_symbols_init();

	class_obexoffset_set(c, calcoffset(t_count_tilde, obex));
	    					
	class_addmethod(c, (method)count_tilde_dsp,			"dsp", A_CANT, 0);		// Bind method "count_tilde_dsp" to the DSP call from MSP
	class_addmethod(c, (method)count_tilde_assist,		"assist", A_CANT,0);	// Bind method "count_tilde_assist" to assistance calls

	class_dspinit(c);														// Setup object's class to work with MSP
	class_register(CLASS_BOX, c);
	count_tilde_class = c;

	return 0;
}
#endif

/************************************************************************************/
// Object Creation Method

static void *count_tilde_new(t_symbol *s, long argc, t_atom *argv)
{
	t_count_tilde *x;
#ifdef PD
	if ((x = (t_count_tilde *)pd_new(count_tilde_class)))
	{
		outlet_new(&x->ob, gensym("signal"));	// Create new signal outlet
		x->value = 0;
	}
#else
	if ((x = (t_count_tilde *)object_alloc(count_tilde_class)))
	{
		dsp_setup((t_pxobject *)x,1);
		outlet_new((t_object *)x, "signal");	// Create signal outlet
		x->value = 0;
	}
#endif
	return (x);
}


/************************************************************************************/
// Methods bound to input/inlets

#ifndef PD
// Method for Assistance Messages
static void count_tilde_assist(t_count_tilde *x, void *b, long msg, long arg, char *dst)
{
	if(msg==1) strcpy(dst, "(signal) input from teabox");	// Inlet
	else if(msg==2) strcpy(dst, "(signal) index");			// Outlet		
}
#endif


// Perform (signal) Method
static t_int *count_tilde_perform(t_int *w)
{
	t_count_tilde *x = (t_count_tilde *)(w[1]);	// Pointer	
	t_float *in = (t_float *)(w[2]);			// Inlet
	t_float *out = (t_float *)(w[3]);			// Outlet
	int n = (int)(w[4]);						// Vector Size
	float value;
	
#ifndef PD
	if (x->ob.z_disabled) goto out;
#endif
	
	while(n--){	
		value = *in++;							// load input sample
		
		x->value++;

		if(value < 0)
			x->value = 0;
		
		*out++ = x->value;			// Output stored count value			
	}
#ifndef PD
out:
#endif
    return (w+5);
}


// DSP Method
#ifdef PD
static void count_tilde_dsp(t_count_tilde *x, t_signal **sp)
#else
static void count_tilde_dsp(t_count_tilde *x, t_signal **sp, short *count)
#endif
{
	dsp_add(count_tilde_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}
