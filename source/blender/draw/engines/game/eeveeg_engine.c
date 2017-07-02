/*
 * Copyright 2016, Blender Foundation.
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
 * Contributor(s): Blender Institute
 *
 */

/** \file eeveeg_engine.c
 *  \ingroup draw_engine
 */

#include "DRW_render.h"

#include "eeveeg_engine.h"
#include "../eevee/eevee_engine.c"

#define GAME_ENGINE "BLENDER_GAME"

DrawEngineType draw_engine_game_type = {
	NULL, NULL,
	N_("Game Engine"),
	&EEVEE_data_size,
	&EEVEE_engine_init,
	&EEVEE_engine_free,
	&EEVEE_cache_init,
	&EEVEE_cache_populate,
	&EEVEE_cache_finish,
	&EEVEE_draw_scene,
	NULL//&EEVEE_draw_scene
};

RenderEngineType DRW_engine_viewport_game_type = {
	NULL, NULL,
	GAME_ENGINE, N_("Game Engine"), RE_INTERNAL | RE_USE_SHADING_NODES,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL,
	&draw_engine_game_type,
	{NULL, NULL, NULL}
};

#undef GAME_ENGINE
