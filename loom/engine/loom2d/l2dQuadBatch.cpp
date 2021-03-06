/*
 * ===========================================================================
 * Loom SDK
 * Copyright 2011, 2012, 2013
 * The Game Engine Company, LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * ===========================================================================
 */

#include "l2dQuadBatch.h"
#include "loom/engine/loom2d/l2dBlendMode.h"
#include "loom/graphics/gfxGraphics.h"

namespace Loom2D
{
Type *QuadBatch::typeQuadBatch = NULL;

void QuadBatch::render(lua_State *L)
{
    // if we have no native texture assigned bail
    if (nativeTextureID == -1)
    {
        return;
    }

    // check the texture info
    GFX::TextureInfo *tinfo = GFX::Texture::getTextureInfo(nativeTextureID);

    if (!tinfo)
    {
        return;
    }

    // apply the parent alpha
    renderState.alpha          = parent ? parent->renderState.alpha * alpha : alpha;
    renderState.clampAlpha();

    // if render state has 0.0 alpha, quad batch is invisible so don't render at all and get out of here now!
    if(renderState.alpha == 0.0f)
    {
        return;
    }

    renderState.clipRect = parent ? parent->renderState.clipRect : Loom2D::Rectangle(0, 0, -1, -1);
    if (renderState.isClipping()) GFX::Graphics::setClipRect((int)renderState.clipRect.x, (int)renderState.clipRect.y, (int)renderState.clipRect.width, (int)renderState.clipRect.height);

    //set blend mode based to be unique or that of our parent
    renderState.blendMode = (blendMode == BlendMode::AUTO && parent) ? parent->renderState.blendMode : blendMode;

    unsigned int blendSrc, blendDst;
    BlendMode::BlendFunction(renderState.blendMode, blendSrc, blendDst);

    // update and get our transformation matrix
    updateLocalTransform();
    
    Matrix mtx;
    getTargetTransformationMatrix(NULL, &mtx);
    
    // quick render and early out of the entire function if the transform is identity and there is no alpha modulation by the render state
    bool isIdentity = mtx.isIdentity();
    if((renderState.alpha == 1.0f) && isIdentity)
    {
        GFX::QuadRenderer::batch(quadData, 4 * numQuads, nativeTextureID, blendEnabled, blendSrc, blendDst, shader);
        return;
    }

    GFX::VertexPosColorTex *v = GFX::QuadRenderer::getQuadVertexMemory(4 * numQuads, nativeTextureID, blendEnabled, blendSrc, blendDst, shader);
    if (!v)
    {
        return;
    }
    GFX::VertexPosColorTex *src = quadData;


    // transform all quads in the batch and submit them to the QuadBatcher
    for (int i = 0; i < numQuads * 4; i++)
    {
        *v = *src;

        // only do matrix transform if the matrix is not identity
        if(!isIdentity)
        {
            lmscalar _x = mtx.a * v->x + mtx.c * v->y + mtx.tx;
            lmscalar _y = mtx.b * v->x + mtx.d * v->y + mtx.ty;

            v->x = (float) _x;
            v->y = (float) _y;
        }

        // modulate vertex alpha by our DisplayObject alpha setting
        if (renderState.alpha != 1.0f)
        {
            //TODO: LOOM-1624
            lmscalar va = ((lmscalar)(v->abgr >> 24)) * renderState.alpha;
            v->abgr = ((uint32_t)va << 24) | (v->abgr & 0x00FFFFFF);
        }

        v++;
        src++;
    }
}
}
