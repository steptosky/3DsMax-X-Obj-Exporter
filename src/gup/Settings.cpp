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

#pragma warning(push, 0)
#include <max.h>
#pragma warning(pop)

#include "Settings.h"
#include "Info.h"
#include "objects/main/MainObjParamsWrapper.h"
#include "common/NodeVisitor.h"

/**************************************************************************************************/
////////////////////////////////////* Constructors/Destructor */////////////////////////////////////
/**************************************************************************************************/

/**************************************************************************************************/
//////////////////////////////////////////* Functions */////////////////////////////////////////////
/**************************************************************************************************/

bool Settings::isSavedAsXplnScene() const {
	return value("xpln_scene", false);
}

void Settings::prepareDataForSave() {
	setSceneVersion(currentVersion());
	setValue("xpln_scene", sceneContainsMainObj());
}

void Settings::setSceneVersion(const sts::SemVersion & version) {
	setValue("verMajor", version.major);
	setValue("verMinor", version.minor);
	setValue("verPatch", version.patch);
}

sts::SemVersion Settings::sceneVersion() const {
	return sts::SemVersion(value("verMajor", uint32_t(0)),
							value("verMinor", uint32_t(0)),
							value("verPatch", uint32_t(0)));
}

sts::SemVersion Settings::currentVersion() {
	return sts::SemVersion(XIO_VERSION_MAJOR, XIO_VERSION_MINOR, XIO_VERSION_PATCH);
}

/**************************************************************************************************/
//////////////////////////////////////////* Functions */////////////////////////////////////////////
/**************************************************************************************************/

bool Settings::sceneContainsMainObj() {
	auto hasMainObj = [](INode * n) ->bool { return !MainObjParamsWrapper::isMainObj(n); };
	return !NodeVisitor::visitChildrenOf(GetCOREInterface()->GetRootNode(), hasMainObj);
}

/**************************************************************************************************/
////////////////////////////////////////////////////////////////////////////////////////////////////
/**************************************************************************************************/