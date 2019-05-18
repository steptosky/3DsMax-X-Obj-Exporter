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

#include "ManipAttrCmd.h"

#pragma warning(push, 0)
#include <3dsmaxport.h>
#pragma warning(pop)

#include <xpln/enums/ECursor.h>
#include "ui-win/Utils.h"
#include "common/Logger.h"
#include "resource/ResHelper.h"
#include "presenters/Commands.h"

namespace ui {
namespace win {

    /**************************************************************************************************/
    //////////////////////////////////////////* Static area *///////////////////////////////////////////
    /**************************************************************************************************/

    INT_PTR CALLBACK ManipAttrCmd::panelProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        ManipAttrCmd * theDlg;
        if (msg == WM_INITDIALOG) {
            theDlg = reinterpret_cast<ManipAttrCmd*>(lParam);
            DLSetWindowLongPtr(hWnd, lParam);
            theDlg->initWindow(hWnd);
        }
        else if (msg == WM_DESTROY) {
            theDlg = DLGetWindowLongPtr<ManipAttrCmd*>(hWnd);
            theDlg->destroyWindow(hWnd);
        }
        else {
            theDlg = DLGetWindowLongPtr<ManipAttrCmd *>(hWnd);
            if (!theDlg) {
                return FALSE;
            }
        }

        //--------------------------------------

        switch (msg) {
            case WM_COMMAND: {
                switch (LOWORD(wParam)) {
                    case BTN_COMMAND: {
                        MSTR str;
                        Utils::getText(theDlg->cEdtCommand, str);
                        str = presenters::Commands::selectData(str);
                        theDlg->cEdtCommand->SetText(str);
                        theDlg->mData.mCommand = xobj::fromMStr(str);
                        theDlg->save();
                        break;
                    }
                    case CMB_CURSOR: {
                        if (HIWORD(wParam) == CBN_SELCHANGE) {
                            theDlg->mData.mCursor = xobj::ECursor::fromUiString(sts::toMbString(theDlg->cCmbCursor.currSelectedText()).c_str());
                            theDlg->save();
                        }
                        break;
                    }
                    default: break;
                }
                break;
            }
            case WM_CUSTEDIT_ENTER: {
                switch (LOWORD(wParam)) {
                    case EDIT_COMMAND: {
                        theDlg->mData.mCommand = sts::toMbString(Utils::getText(theDlg->cEdtCommand));
                        theDlg->save();
                        break;
                    }
                    case EDIT_TOOLTIP: {
                        theDlg->mData.mToolType = sts::toMbString(Utils::getText(theDlg->cEdtToolType));
                        theDlg->save();
                        break;
                    }
                    default: break;
                }
                break;
            }
            default: break;
        }
        return 0;
    }

    /**************************************************************************************************/
    ////////////////////////////////////* Constructors/Destructor */////////////////////////////////////
    /**************************************************************************************************/

    ManipAttrCmd::ManipAttrCmd(MdManip * modelData)
        : mModelData(modelData) {
        assert(mModelData);
    }

    ManipAttrCmd::~ManipAttrCmd() {
        ManipAttrCmd::destroy();
    }

    /**************************************************************************************************/
    ///////////////////////////////////////////* Functions *////////////////////////////////////////////
    /**************************************************************************************************/

    void ManipAttrCmd::create(HWND inParent) {
        assert(inParent);
        mHwnd.setup(CreateDialogParam(ResHelper::hInstance,
                                      MAKEINTRESOURCE(ROLL_MANIP_COMMAND),
                                      inParent, panelProc,
                                      reinterpret_cast<LPARAM>(this)));
        assert(mHwnd);
        if (mHwnd) {
            toWindow();
            mHwnd.show(true);
        }
        else {
            LError << WinCode(GetLastError());
        }
    }

    void ManipAttrCmd::destroy() {
        if (mHwnd) {
            BOOL res = DestroyWindow(mHwnd.hwnd());
            if (!res) {
                LError << WinCode(GetLastError());
            }
            mHwnd.release();
        }
    }

    RECT ManipAttrCmd::rect() const {
        RECT r{0, 0, 0, 0};
        if (mHwnd) {
            r = mHwnd.rect();
        }
        return r;
    }

    void ManipAttrCmd::move(const POINT & point) {
        if (mHwnd) {
            mHwnd.move(point);
        }
    }

    /**************************************************************************************************/
    //////////////////////////////////////////* Functions */////////////////////////////////////////////
    /**************************************************************************************************/

    void ManipAttrCmd::setManip(const std::optional<xobj::AttrManip> & manip) {
        assert(manip);
        const auto data = std::get_if<xobj::AttrManipCmd>(&manip->mType);
        if (!data) {
            const xobj::EManipulator type = std::visit([](auto && m) { return m.mType; }, manip->mType);
            LError << "Incorrect manipulator type: " << type.toString();
            return;
        }
        mData = *data;
    }

    /**************************************************************************************************/
    ///////////////////////////////////////////* Functions *////////////////////////////////////////////
    /**************************************************************************************************/

    void ManipAttrCmd::initWindow(HWND hWnd) {
        cBtnCommand.setup(hWnd, BTN_COMMAND);
        cEdtCommand = GetICustEdit(GetDlgItem(hWnd, EDIT_COMMAND));
        cEdtToolType = GetICustEdit(GetDlgItem(hWnd, EDIT_TOOLTIP));
        cCmbCursor.setup(hWnd, CMB_CURSOR);

        for (auto & curr : xobj::ECursor::list()) {
            cCmbCursor.addItem(sts::toString(curr.toUiString()));
        }
        cCmbCursor.setCurrSelected(0);
    }

    void ManipAttrCmd::destroyWindow(HWND /*hWnd*/) {
        cBtnCommand.release();
        cCmbCursor.release();
        ReleaseICustEdit(cEdtCommand);
        ReleaseICustEdit(cEdtToolType);
    }

    void ManipAttrCmd::toWindow() {
        cEdtCommand->SetText(xobj::toMStr(mData.mCommand));
        cEdtToolType->SetText(xobj::toMStr(mData.mToolType));
        cCmbCursor.setCurrSelected(sts::toString(mData.mCursor.toUiString()));
    }

    /********************************************************************************************************/
    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    /********************************************************************************************************/
}
}
