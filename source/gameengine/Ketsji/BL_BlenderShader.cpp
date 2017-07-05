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
#  include "../blender/gpu/intern/gpu_codegen.h"
}

BL_BlenderShader::BL_BlenderShader(KX_Scene *scene, struct Material *ma, int lightlayer)
	:m_blenderScene(scene->GetBlenderScene()),
	m_mat(ma),
	m_lightLayer(lightlayer),
	m_alphaBlend(GPU_BLEND_SOLID),
	m_shGroup(nullptr)
{
	ReloadMaterial(scene);
}

BL_BlenderShader::~BL_BlenderShader()
{
	/* In the case the material use nodes, we create a DRWShadingGroup,
	 * so we need to free it at game engine exit. But if the material
	 * doesn't use nodes, we use existing shading group in eevee so if
	 * we free it, it causes a crash at game engine exit.
	 */
	if (m_shGroup && m_mat && m_mat->use_nodes && m_mat->nodetree) {
		DRW_shgroup_free(m_shGroup);
	}
}

const RAS_Rasterizer::AttribLayerList BL_BlenderShader::GetAttribLayers(const RAS_MeshObject::LayersInfo& layersInfo) const
{
	RAS_Rasterizer::AttribLayerList attribLayers;

	return attribLayers;
}

bool BL_BlenderShader::Ok() const
{
	return (m_gpuShader != nullptr);
}

void BL_BlenderShader::ReloadMaterial(KX_Scene *scene)
{
	if (m_shGroup) {
		DRW_shgroup_free(m_shGroup);
	}

	if (m_mat) {
		EEVEE_SceneLayerData *sldata = scene->GetSceneLayerData();
		EEVEE_Data *edata = EEVEE_engine_data_get();
		if (m_mat->use_nodes && m_mat->nodetree) {
			GPUMaterial *mat = EEVEE_material_mesh_get(m_blenderScene, m_mat, false, false);
			m_shGroup = DRW_shgroup_material_create(mat, nullptr);
			GPUPass *pass = GPU_material_get_pass(mat);
			m_gpuShader = GPU_pass_shader(pass);
		}
		else {
			bool use_ao = edata->stl->effects->use_ao;
			bool use_bent_normals = edata->stl->effects->use_bent_normals;
			m_shGroup = EEVEE_default_shading_group_get(sldata, edata, false, false, use_ao, use_bent_normals);
			int options = VAR_MAT_MESH;

			if (use_ao) options |= VAR_MAT_AO;
			if (use_bent_normals) options |= VAR_MAT_BENT;

			EEVEE_UtilData *udata = scene->GetUtilData();
			m_gpuShader = udata->default_lit[options];

			float *color_p = &m_mat->r;
			float *metal_p = &m_mat->ray_mirror;
			float *spec_p = &m_mat->spec;
			float *rough_p = &m_mat->gloss_mir;

			DRW_shgroup_uniform_vec3(m_shGroup, "basecol", color_p, 1);
			DRW_shgroup_uniform_float(m_shGroup, "metallic", metal_p, 1);
			DRW_shgroup_uniform_float(m_shGroup, "specular", spec_p, 1);
			DRW_shgroup_uniform_float(m_shGroup, "roughness", rough_p, 1);
		}
		EEVEE_add_standard_uniforms_game(m_shGroup, sldata, edata);
	}
	else {
		m_shGroup = nullptr;
	}

	ParseAttribs();
}

void BL_BlenderShader::SetProg(bool enable, double time, RAS_Rasterizer *rasty)
{
	if (enable && m_shGroup) {
		DRW_draw_shgroup(m_shGroup, (DRWState)(
			DRW_STATE_WRITE_DEPTH |
			DRW_STATE_DEPTH_LESS |
			DRW_STATE_CULL_BACK |
			DRW_STATE_WRITE_COLOR));
	}
	else {
		GPU_shader_unbind();
	}
}

void BL_BlenderShader::ParseAttribs()
{
}

void BL_BlenderShader::SetAttribs(RAS_Rasterizer *ras)
{
}

void BL_BlenderShader::Update(RAS_MeshSlot *ms, RAS_Rasterizer *rasty)
{
	ms->SetGpuShader(m_gpuShader);

	DRW_draw_geometry_prepare(m_shGroup, (float(*)[4])ms->m_meshUser->GetMatrix(), nullptr, nullptr);

	m_alphaBlend = GPU_BLEND_SOLID;
}

bool BL_BlenderShader::UseInstancing() const
{
	return m_mat->shade_flag & MA_INSTANCING;
}

void BL_BlenderShader::ActivateInstancing(void *matrixoffset, void *positionoffset, void *coloroffset, unsigned int stride)
{
	/*if (Ok()) {
		GPU_material_bind_instancing_attrib(m_gpuMat, matrixoffset, positionoffset, coloroffset, stride);
	}*/
}

void BL_BlenderShader::DesactivateInstancing()
{
	/*if (Ok()) {
		GPU_material_unbind_instancing_attrib(m_gpuMat);
	}*/
}

int BL_BlenderShader::GetAlphaBlend()
{
	return m_alphaBlend;
}
