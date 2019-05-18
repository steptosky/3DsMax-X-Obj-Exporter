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

#include "ManipAttrCmdAxis.h"

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

    INT_PTR CALLBACK ManipAttrCmdAxis::panelProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        ManipAttrCmdAxis * theDlg;
        if (msg == WM_INITDIALOG) {
            theDlg = reinterpret_cast<ManipAttrCmdAxis*>(lParam);
            DLSetWindowLongPtr(hWnd, lParam);
            theDlg->initWindow(hWnd);
        }
        else if (msg == WM_DESTROY) {
            theDlg = DLGetWindowLongPtr<ManipAttrCmdAxis*>(hWnd);
            theDlg->destroyWindow(hWnd);
        }
        else {
            theDlg = DLGetWindowLongPtr<ManipAttrCmdAxis *>(hWnd);
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
                        Utils::getText(theDlg->cEdtCmdPos, str);
                        str = presenters::Commands::selectData(str);
                        theDlg->cEdtCmdPos->SetText(str);
                        theDlg->mData.mPosCommand = xobj::fromMStr(str);
                        theDlg->save();
                        break;
                    }
                    case BTN_COMMAND2: {
                        MSTR str;
                        Utils::getText(theDlg->cEdtCmdNeg, str);
                        str = presenters::Commands::selectData(str);
                        theDlg->cEdtCmdNeg->SetText(str);
                        theDlg->mData.mNegCommand = xobj::fromMStr(str);
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
                        theDlg->mData.mPosCommand = sts::toMbString(Utils::getText(theDlg->cEdtCmdPos));
                        theDlg->save();
                        break;
                    }
                    case EDIT_COMMAND2: {
                        theDlg->mData.mNegCommand = sts::toMbString(Utils::getText(theDlg->cEdtCmdNeg));
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
            case CC_SPINNER_CHANGE: {
                switch (LOWORD(wParam)) {
                    case SPN_X: {
                        theDlg->mData.mDirX = theDlg->mSpnX->GetFVal();
                        theDlg->save();
                        break;
                    }
                    case SPN_Y: {
                        theDlg->mData.mDirY = theDlg->mSpnY->GetFVal();
                        theDlg->save();
                        break;
                    }
                    case SPN_Z: {
                        theDlg->mData.mDirZ = theDlg->mSpnZ->GetFVal();
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

    ManipAttrCmdAxis::ManipAttrCmdAxis(MdManip * modelData)
        : mModelData(modelData) {
        assert(mModelData);
    }

    ManipAttrCmdAxis::~ManipAttrCmdAxis() {
        ManipAttrCmdAxis::destroy();
    }

    /**************************************************************************************************/
    ///////////////////////////////////////////* Functions *////////////////////////////////////////////
    /**************************************************************************************************/

    void ManipAttrCmdAxis::create(HWND inParent) {
        assert(inParent);
        mHwnd.setup(CreateDialogParam(ResHelper::hInstance,
                                      MAKEINTRESOURCE(ROLL_MANIP_COMMAND_AXIS),
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

    void ManipAttrCmdAxis::destroy() {
        if (mHwnd) {
            BOOL res = DestroyWindow(mHwnd.hwnd());
            if (!res) {
                LError << WinCode(GetLastError());
            }
            mHwnd.release();
        }
    }

    RECT ManipAttrCmdAxis::rect() const {
        RECT r{0, 0, 0, 0};
        if (mHwnd) {
            r = mHwnd.rect();
        }
        return r;
    }

    void ManipAttrCmdAxis::move(const POINT & point) {
        if (mHwnd) {
            mHwnd.move(point);
        }
    }

    /**************************************************************************************************/
    //////////////////////////////////////////* Functions */////////////////////////////////////////////
    /**************************************************************************************************/

    void ManipAttrCmdAxis::setManip(const std::optional<xobj::AttrManip> & manip) {
        assert(manip);
        const auto data = std::get_if<xobj::AttrManipCmdAxis>(&manip->mType);
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

    void ManipAttrCmdAxis::initWindow(HWND hWnd) {
        mSpnX = SetupFloatSpinner(hWnd, SPN_X, SPN_X_EDIT, -10000.0f, 10000.0f, 0.0f, 0.1f);
        mSpnY = SetupFloatSpinner(hWnd, SPN_Y, SPN_Y_EDIT, -10000.0f, 10000.0f, 0.0f, 0.1f);
        mSpnZ = SetupFloatSpinner(hWnd, SPN_Z, SPN_Z_EDIT, -10000.0f, 10000.0f, 0.0f, 0.1f);

        cBtnCmdNeg.setup(hWnd, BTN_COMMAND2);
        cEdtCmdNeg = GetICustEdit(GetDlgItem(hWnd, EDIT_COMMAND2));
        cBtnCmdPos.setup(hWnd, BTN_COMMAND);
        cEdtCmdPos = GetICustEdit(GetDlgItem(hWnd, EDIT_COMMAND));

        cEdtToolType = GetICustEdit(GetDlgItem(hWnd, EDIT_TOOLTIP));
        cCmbCursor.setup(hWnd, CMB_CURSOR);

        for (auto & curr : xobj::ECursor::list()) {
            cCmbCursor.addItem(sts::toString(curr.toUiString()));
        }
        cCmbCursor.setCurrSelected(0);
    }

    void ManipAttrCmdAxis::destroyWindow(HWND /*hWnd*/) {
        ReleaseISpinner(mSpnX);
        ReleaseISpinner(mSpnY);
        ReleaseISpinner(mSpnZ);
        cBtnCmdNeg.release();
        ReleaseICustEdit(cEdtCmdNeg);
        cBtnCmdPos.release();
        ReleaseICustEdit(cEdtCmdPos);
        ReleaseICustEdit(cEdtToolType);
        cCmbCursor.release();
    }

    void ManipAttrCmdAxis::toWindow() {
        mSpnX->SetValue(mData.mDirX, FALSE);
        mSpnY->SetValue(mData.mDirY, FALSE);
        mSpnZ->SetValue(mData.mDirZ, FALSE);
        cEdtCmdPos->SetText(xobj::toMStr(mData.mPosCommand));
        cEdtCmdNeg->SetText(xobj::toMStr(mData.mNegCommand));
        cEdtToolType->SetText(xobj::toMStr(mData.mToolType));
        cCmbCursor.setCurrSelected(sts::toString(mData.mCursor.toUiString()));
    }

    /********************************************************************************************************/
    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    /********************************************************************************************************/
}
}
