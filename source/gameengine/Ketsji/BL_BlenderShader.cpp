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
* ***** END GPL LICENSE BLOCK *****
*/

/** \file gameengine/Ketsji/BL_BlenderShader.cpp
*  \ingroup ketsji
*/

#include "DNA_material_types.h"
#include "DNA_scene_types.h"

#include "BKE_DerivedMesh.h"

#include "GPU_material.h"
#include "GPU_shader.h"
#include "GPU_extensions.h"


#include "BL_BlenderShader.h"

#include "RAS_BucketManager.h"
#include "RAS_MeshObject.h"
#include "RAS_MeshUser.h"
#include "RAS_Rasterizer.h"

#include "KX_Scene.h"

extern "C" {
#  include "eevee_engine.h"
#  include "DRW_engine.h"
#  include "DRW_render.h"
}

extern "C" {
	extern char datatoc_ltc_lib_glsl[];
	extern char datatoc_bsdf_common_lib_glsl[];
	extern char datatoc_bsdf_direct_lib_glsl[];
	extern char datatoc_lit_surface_frag_glsl[];
	extern char datatoc_lit_surface_vert_glsl[];
	extern char datatoc_irradiance_lib_glsl[];
	extern char datatoc_octahedron_lib_glsl[];
	extern char datatoc_ambient_occlusion_lib_glsl[];
}

BL_BlenderShader::BL_BlenderShader(KX_Scene *scene, struct Material *ma, int lightlayer)
	:m_blenderScene(scene->GetBlenderScene()),
	m_mat(ma),
	m_lightLayer(lightlayer),
	m_alphaBlend(GPU_BLEND_SOLID),
	m_gpuMat(nullptr),
	m_shGroup(nullptr)
{
	ReloadMaterial(scene);
}

BL_BlenderShader::~BL_BlenderShader()
{
	if (m_shGroup) {
		DRW_shgroup_free(m_shGroup);
	}
}

const RAS_Rasterizer::AttribLayerList BL_BlenderShader::GetAttribLayers(const RAS_MeshObject::LayersInfo& layersInfo) const
{
	RAS_Rasterizer::AttribLayerList attribLayers;
	GPUVertexAttribs attribs;
	GPU_material_vertex_attributes(m_gpuMat, &attribs);

	for (unsigned int i = 0; i < attribs.totlayer; ++i) {
		if (attribs.layer[i].type == CD_MTFACE || attribs.layer[i].type == CD_MCOL) {
			const char *attribname = attribs.layer[i].name;
			if (strlen(attribname) == 0) {
				// The color or uv layer is not specified, then use the active color or uv layer.
				if (attribs.layer[i].type == CD_MTFACE) {
					attribLayers[attribs.layer[i].glindex] = layersInfo.activeUv;
				}
				else {
					attribLayers[attribs.layer[i].glindex] = layersInfo.activeColor;
				}
				continue;
			}

			for (RAS_MeshObject::LayerList::const_iterator it = layersInfo.layers.begin(), end = layersInfo.layers.end();
				it != end; ++it) {
				const RAS_MeshObject::Layer& layer = *it;
				bool found = false;
				if (attribs.layer[i].type == CD_MTFACE && layer.face && layer.name == attribname) {
					found = true;
				}
				else if (attribs.layer[i].type == CD_MCOL && layer.color && layer.name == attribname) {
					found = true;
				}
				if (found) {
					attribLayers[attribs.layer[i].glindex] = layer.index;
					break;
				}
			}
		}
	}

	return attribLayers;
}

bool BL_BlenderShader::Ok() const
{
	return (m_gpuMat != nullptr);
}

void BL_BlenderShader::ReloadMaterial(KX_Scene *scene)
{
	m_gpuMat = EEVEE_material_mesh_get(m_blenderScene, m_mat, false, false);

	if (m_shGroup) {
		DRW_shgroup_free(m_shGroup);
	}
	m_shGroup = DRW_shgroup_material_create(m_gpuMat, nullptr);
	EEVEE_add_standard_uniforms_game(m_shGroup, scene->GetSceneLayerData(), EEVEE_engine_data_get());

	ParseAttribs();
}

void BL_BlenderShader::SetProg(bool enable, double time, RAS_Rasterizer *rasty)
{
	if (Ok()) {
		if (enable) {

			DRW_draw_shgroup(m_shGroup, (DRWState)(
				DRW_STATE_WRITE_DEPTH |
				DRW_STATE_DEPTH_LESS |
				// DRW_STATE_CULL_BACK |
				DRW_STATE_WRITE_COLOR));
		}
		else {
			GPU_material_unbind(m_gpuMat);
		}
	}
}

void BL_BlenderShader::ParseAttribs()
{
	if (!Ok()) {
		return;
	}

	GPUVertexAttribs attribs;
	GPU_material_vertex_attributes(m_gpuMat, &attribs);

	m_attribs.clear();

	for (unsigned short i = 0; i < attribs.totlayer; ++i) {
		const int type = attribs.layer[i].type;
		const int glindex = attribs.layer[i].glindex;
		if (type == CD_MTFACE) {
			m_attribs.emplace_back(glindex, RAS_Rasterizer::RAS_TEXCO_UV);
		}
		else if (type == CD_TANGENT) {
			m_attribs.emplace_back(glindex, RAS_Rasterizer::RAS_TEXTANGENT);
		}
		else if (type == CD_ORCO) {
			m_attribs.emplace_back(glindex, RAS_Rasterizer::RAS_TEXCO_ORCO);
		}
		else if (type == CD_NORMAL) {
			m_attribs.emplace_back(glindex, RAS_Rasterizer::RAS_TEXCO_NORM);
		}
		else if (type == CD_MCOL) {
			m_attribs.emplace_back(glindex, RAS_Rasterizer::RAS_TEXCO_VCOL);
		}
	}
}

void BL_BlenderShader::SetAttribs(RAS_Rasterizer *ras)
{
	if (!Ok()) {
		return;
	}

	ras->ClearTexCoords();
	ras->SetAttribs(m_attribs);
}

void BL_BlenderShader::Update(RAS_MeshSlot *ms, RAS_Rasterizer *rasty)
{
	ms->SetGpuMat(m_gpuMat);

	float *obcol = (float *)ms->m_meshUser->GetColor().getValue();

	DRW_draw_geometry_prepare(m_shGroup, (float(*)[4])ms->m_meshUser->GetMatrix(), nullptr, nullptr);

	m_alphaBlend = GPU_material_alpha_blend(m_gpuMat, obcol);
}

bool BL_BlenderShader::UseInstancing() const
{
	return m_mat->shade_flag & MA_INSTANCING;
}

void BL_BlenderShader::ActivateInstancing(void *matrixoffset, void *positionoffset, void *coloroffset, unsigned int stride)
{
	if (Ok()) {
		GPU_material_bind_instancing_attrib(m_gpuMat, matrixoffset, positionoffset, coloroffset, stride);
	}
}

void BL_BlenderShader::DesactivateInstancing()
{
	if (Ok()) {
		GPU_material_unbind_instancing_attrib(m_gpuMat);
	}
}

int BL_BlenderShader::GetAlphaBlend()
{
	return m_alphaBlend;
}
