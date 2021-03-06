/*
 * ***** BEGIN GPL LICENSE BLOCK *****
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Contributor(s): Tristan Porteries.
 *
 * ***** END GPL LICENSE BLOCK *****
 */

/** \file KX_2DFilter.h
*  \ingroup ketsji
*/

#ifndef __KX_2DFILTER_OFFSCREEN_H__
#define __KX_2DFILTER_OFFSCREEN_H__

#include "RAS_2DFilterOffScreen.h"
#include "EXP_Value.h"

class KX_2DFilterOffScreen : public CValue, public RAS_2DFilterOffScreen
{
	Py_Header
public:
	KX_2DFilterOffScreen(unsigned short colorSlots, Flag flag, unsigned int width, unsigned int height, RAS_Rasterizer::HdrType hdr);
	virtual ~KX_2DFilterOffScreen();

	virtual std::string GetName();

#ifdef WITH_PYTHON

	static PyObject *pyattr_get_width(PyObjectPlus *self_v, const KX_PYATTRIBUTE_DEF *attrdef);
	static PyObject *pyattr_get_height(PyObjectPlus *self_v, const KX_PYATTRIBUTE_DEF *attrdef);
	static PyObject *pyattr_get_colorBindCodes(PyObjectPlus *self_v, const KX_PYATTRIBUTE_DEF *attrdef);
	static PyObject *pyattr_get_depthBindCode(PyObjectPlus *self_v, const KX_PYATTRIBUTE_DEF *attrdef);

#endif
};

#endif  // __KX_2DFILTER_OFFSCREEN_H__
