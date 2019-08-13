/*
**  Copyright(C) 2019, StepToSky
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

#pragma once
#include "models/MdAnimRot.h"
#include "ui-win/controls/CheckBox.h"
#include "ui-win/controls/List.h"
#include "ui-win/controls/BaseProc.h"

#pragma warning(push, 0)
#include <notify.h>
#pragma warning(pop)

namespace ui {
namespace win {

    /********************************************************************************************************/
    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    /********************************************************************************************************/

    class AnimRotateAxisView : public max::ctrl::BaseProc {
    public:

        explicit AnimRotateAxisView(MdAnimRot::eAxis axis);
        virtual ~AnimRotateAxisView();

        bool create(HWND parent);
        void destroy();

        void setWindowPos(const RECT & size);
        RECT clientRect() const;
        void active(bool state);
        bool isActive() const;
    private:

        INT_PTR panelProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override;
        void initWindow(HWND hWnd) override;
        void destroyWindow(HWND hWnd) override;
        //----------------------------------------------------

        MdAnimRot mData;
        void toWindow();
        void enableControls();
        void disableControls();
        void clearValues();
        void makeUiList();
        void setEnable();
        void setReverseEnable();
        void setLoopEnable();
        void setDataref();
        void loopSpnChanged();
        void valueSpnChanged();
        void selectionChanged();
        void reverseValues();
        void calculateValues();
        void setDataRefValueAsToolType();

        void loadSelection();

        int mCurrSelected = 0;
        ctrl::Base cStcName;
        ctrl::Base cBtnDataRef;
        ctrl::Base cBtnReverseValue;
        ctrl::Base cBtnCalculateValue;
        ctrl::Base cBtnUpdate;
        ctrl::Base cStcValue1;
        ctrl::Base cStcValue2;
        ctrl::List cListKeys;
        ctrl::CheckBox cChkEnable;
        ctrl::CheckBox cChkReverse;
        ctrl::CheckBox cChkLoop;
        ICustEdit * cEditDataRef = nullptr;
        ISpinnerControl * cSpnValue = nullptr;
        ISpinnerControl * cSpnLoopValue = nullptr;

        Interface * mIp = nullptr;
        bool mIsActive = false;
        //-------------------------------------------------------------------------

        void registerCallbacks();
        void unRegisterCallbacks();

        static void slotSelectionChange(void * param, NotifyInfo *);
        static void slotObjectsDeleted(void * param, NotifyInfo * info);
        static void slotAnimationModeOff(void * param, NotifyInfo *);
        //-------------------------------------------------------------------------

    };

    /********************************************************************************************************/
    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    /********************************************************************************************************/
}
}
