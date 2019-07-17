/*
**  Copyright(C) 2017, StepToSky
**
**  Redistribution and use in source and binary forms, with or without
**  modification, are permitted provided that the following conditions are met:
**
**  1.Redistributions of source code must retain the above copyright notice, this
**    list of conditions and the following disclaimer.
**  2.Redistributions in binary form must reproduce the above copyright notice,
**    this list of conditions and the following disclaimer in the documentation
**    and / or other materials provided with the distribution.
**  3.Neither the name of StepToSky nor the names of its contributors
**    may be used to endorse or promote products derived from this software
**    without specific prior written permission.
**
**  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
**  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
**  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
**  DISCLAIMED.IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
**  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
**  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
**  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
**  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
**  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
**  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
**  Contacts: www.steptosky.com
*/

#include <cassert>

#include "ConverterToXpln.h"
#include "common/Logger.h"

#include "ConverterDummy.h"
#include "ConverterLine.h"
#include "ConverterMesh.h"
#include "ConverterMain.h"
#include "ConverterLight.h"
#include "ConverterATTR.h"
#include "ConverterAnimVisibility.h"
#include "ConverterAnimTranslate.h"
#include "ConverterAnimRotate.h"
#include "ConverterUtils.h"
#include "ConverterSmoke.h"
#include "ConverterLod.h"

#include <xpln/obj/ObjMain.h>
#include <xpln/obj/ObjMesh.h>
#include <xpln/obj/ObjAbstractLight.h>
#include <xpln/obj/ObjDummy.h>
#include <xpln/obj/ObjSmoke.h>

#include "objects/lod/LodObjParamsWrapper.h"
#include "objects/main/MainObjParamsWrapper.h"
#include "common/String.h"

#include "ExportParams.h"
#include "ImportParams.h"

/**************************************************************************************************/
////////////////////////////////////* Constructors/Destructor */////////////////////////////////////
/**************************************************************************************************/

ConverterToXpln::ConverterToXpln() {
    mIp = GetCOREInterface();

    mXObjMain = nullptr;
    mMainObj = nullptr;
}

/**************************************************************************************************/
///////////////////////////////////////////* Functions *////////////////////////////////////////////
/**************************************************************************************************/

bool ConverterToXpln::toXpln(MainObjParamsWrapper * mainNode, xobj::ObjMain & xObjMain) {
    assert(mainNode);
    mXObjMain = &xObjMain;
    mMainObj = mainNode;

    //-------------------------------------------------------------------------
    ExportParams exportParams;
    exportParams.mCurrTime = mainNode->timeValue();

    if (mainNode->isManualScale()) {
        // TODO Needs implementation of auto-scale value relative system units
        exportParams.mScale = mainNode->scale();
    }

    if (exportParams.mScale < 0.00001) {
        CLError << "<" << xObjMain.objectName() << "> has too small scale <" << exportParams.mScale << ">";
        return false;
    }
    //-------------------------------------------------------------------------

    ConverterMain::toXpln(mainNode->node(), xObjMain, exportParams);
    mLods.clear();
    if (!collectLods(mainNode->node(), mainNode->node(), mLods)) {
        return false;
    }

    if (mLods.empty()) {
        mLods.emplace_back(mainNode->node());
    }

    for (auto currLodNode : mLods) {
        if (currLodNode->NumberOfChildren() == 0) {
            CLWarning << "The lod: <" << sts::toMbString(currLodNode->GetName()) << "> does not have any objects.";
            continue;
        }
        //-------------------------------------------------------------------------
        xobj::ObjLodGroup & lod = xObjMain.addLod();
        xobj::Transform & currObjTransform = lod.transform();

        if (mainNode->node() != currLodNode) {
            ConverterLod::toXpln(currLodNode, lod, exportParams);
        }
        //-------------------------------------------------------------------------
        Matrix3 ttm = Inverse(currLodNode->GetNodeTM(exportParams.mCurrTime));
        ttm.Scale(Point3(exportParams.mScale, exportParams.mScale, exportParams.mScale), TRUE);
        //-------------------------------------------------------------------------
        ConverterUtils::toXTransform(ttm, currObjTransform);

        const auto numChildren = currLodNode->NumberOfChildren();
        for (int idx = 0; idx < numChildren; ++idx) {
            if (!processNode(currLodNode->GetChildNode(idx), &currObjTransform, exportParams)) {
                return false;
            }
        }
        //-------------------------------------------------------------------------
    }
    return true;
}

/**************************************************************************************************/
///////////////////////////////////////////* Functions *////////////////////////////////////////////
/**************************************************************************************************/

bool ConverterToXpln::processNode(INode * node, xobj::Transform * xTransform, const ExportParams & params) const {
    xobj::Transform & tr = xTransform->newChild(sts::toMbString(node->GetName()).c_str());
    ConverterUtils::toXTransform(node->GetNodeTM(mMainObj->timeValue()), tr);
    //-------------------------------------------------------------------------
    // animation
    DbgAssert(node);
    if (mMainObj->isAnimationExport()) {
        ConverterAnimVisibility::toXpln(*node, tr, params);
        ConverterAnimTranslate::toXpln(*node, tr, params);
        ConverterAnimRotate::toXpln(*node, tr, params);
    }
    //-------------------------------------------------------------------------
    // translate object
    static ObjAbstractList xObjList;
    toXpln(node, Matrix3(1), xObjList, params);
    for (auto curr : xObjList) {
        tr.addObject(curr);
    }
    //-------------------------------------------------------------------------
    // translate children
    const auto numChildren = node->NumberOfChildren();
    for (int idx = 0; idx < numChildren; ++idx) {
        if (!processNode(node->GetChildNode(idx), &tr, params)) {
            return false;
        }
    }
    //-------------------------------------------------------------------------
    return true;
}

//-------------------------------------------------------------------------

void ConverterToXpln::toXpln(INode * inNode, const Matrix3 & baseTm,
                             ObjAbstractList & outList, const ExportParams & params) const {
    outList.clear();

    if (mMainObj->isMeshExport()) {
        xobj::ObjAbstract * xObj = ConverterMesh::toXpln(inNode, params);
        if (xObj) {
            ConverterAttr::toXpln(*xObj, inNode, params);
            outList.emplace_back(xObj);
            return;
        }
    }
    if (mMainObj->isLinesExport()) {
        outList = ConverterLine::toXpln(inNode, baseTm, params);
        if (!outList.empty()) {
            return;
        }
    }
    if (mMainObj->isLightsExport()) {
        xobj::ObjAbstract * xObj = ConverterLight::toXpln(inNode, params);
        if (xObj) {
            ConverterAttr::toXpln(*xObj, inNode, params);
            outList.emplace_back(xObj);
            return;
        }
    }

    xobj::ObjSmoke * xSmokeObj = ConverterSmoke::toXpln(inNode, params);
    if (xSmokeObj) {
        outList.emplace_back(xSmokeObj);
        return;
    }

    xobj::ObjAbstract * xDummyObj = ConverterDummy::toXpln(inNode, params);
    if (xDummyObj) {
        ConverterAttr::toXpln(*xDummyObj, inNode, params);
        outList.emplace_back(xDummyObj);
        return;
    }
}

/**************************************************************************************************/
///////////////////////////////////////////* Functions *////////////////////////////////////////////
/**************************************************************************************************/

bool ConverterToXpln::collectLods(INode * ownerNode, INode * currNode, std::vector<INode*> & outLods) {
    if (LodObjParamsWrapper::isLodObj(currNode)) {
        if (currNode->GetParentNode() != ownerNode) {
            CLError << "The lod object <"
                    << sts::toMbString(currNode->GetName()) << "> is not linked to <"
                    << sts::toMbString(ownerNode->GetName()) << ">";
            return false;
        }
        outLods.emplace_back(currNode);
    }

    const auto count = currNode->NumberOfChildren();
    for (int i = 0; i < count; ++i) {
        if (!collectLods(ownerNode, currNode->GetChildNode(i), outLods)) {
            return false;
        }
    }
    return true;
}

/**************************************************************************************************/
////////////////////////////////////////////////////////////////////////////////////////////////////
/**************************************************************************************************/
