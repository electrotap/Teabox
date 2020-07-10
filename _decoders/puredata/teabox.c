/*
 * Teabox library for Pure Data
 * Written by Olaf Matthes
 * Copyright © 2007.
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

/* this has to be library because we want filenames like teabox.bits~ */

#include "m_pd.h"


typedef struct _teabox_library
{
     t_object x_obj;
} t_teabox_library;

static t_class *teabox_library_class;

	/* objects */
void teabox_tilde_setup();
void teabox_count_tilde_setup();
void teabox_bits_tilde_setup();

static void *teabox_library_new(t_symbol* s)
{
    t_teabox_library *x = (t_teabox_library *)pd_new(teabox_library_class);
    return (x);
}

void teabox_setup(void) 
{
	teabox_library_class = class_new(gensym("teabox"), (t_newmethod)teabox_library_new, 0,
    	sizeof(t_teabox_library), 0,0);

	teabox_tilde_setup();
	teabox_count_tilde_setup();
	teabox_bits_tilde_setup();
	post("teabox library v1.0");
}
