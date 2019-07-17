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

#pragma once

#pragma warning(push, 0)
#include <max.h>
#include <IPathConfigMgr.h>
#pragma warning(pop)

#include <exception>
#include <cctype>
#include "ui-win/DlgDcView.h"
#include "common/Config.h"
#include "gup/ObjCommon.h"

using namespace std::string_literals;

namespace presenters {

/**************************************************************************************************/
////////////////////////////////////* Constructors/Destructor */////////////////////////////////////
/**************************************************************************************************/

template<typename T>
Dc<T>::Dc(IView * view)
    : mView(view) {

    DbgAssert(mView);

    view->sigKeyChanged = std::bind(&Dc<T>::onKeyChanged, this, std::placeholders::_1, std::placeholders::_2);
    view->sigCurrFileChanged = std::bind(&Dc<T>::onCurrFileChanged, this, std::placeholders::_1);
    view->sigReady = std::bind(&Dc<T>::onViewReady, this);
    view->sigSearchKeyChanged = std::bind(&Dc<T>::onSearchKeyChanged, this, std::placeholders::_1);

    ObjCommon::instance()->pSettings.onProjectSettingsChanged.connect(this, &Dc<T>::onSettingsChanged);

    RegisterNotification(onFileOpened, this, NOTIFY_FILE_POST_OPEN);
    RegisterNotification(onSystemReset, this, NOTIFY_SYSTEM_POST_RESET);
    RegisterNotification(onSystemNew, this, NOTIFY_SYSTEM_POST_NEW);

    auto config = Config::instance();
    config->sigSimDirChanged.connect(this, &Dc<T>::onSimDirChanged);

    loadSimDatarefs();
    loadProjectDatarefs();
}

template<typename T>
Dc<T>::~Dc() {
    UnRegisterNotification(onFileOpened, this, NOTIFY_FILE_POST_OPEN);
    UnRegisterNotification(onSystemReset, this, NOTIFY_SYSTEM_POST_RESET);
    UnRegisterNotification(onSystemNew, this, NOTIFY_SYSTEM_POST_NEW);
    delete mView;
}

/**************************************************************************************************/
//////////////////////////////////////////* Functions */////////////////////////////////////////////
/**************************************************************************************************/

template<typename T>
MStr Dc<T>::selectData(const MStr & current) {
    if (!Dc<T>::isInstanced()) {
        init(new ui::win::DlgDcView<IView>());
    }
    Dc<T>::mInstance->mCurrKey = current;
    Dc<T>::mInstance->mView->open();
    return Dc<T>::mInstance->mCurrKey;
}

/**************************************************************************************************/
//////////////////////////////////////////* Functions */////////////////////////////////////////////
/**************************************************************************************************/

template<typename T>
MaxSDK::Util::Path Dc<T>::simDataFile(Config & config) {
    if constexpr (std::is_same<md::DatarefsFile, T>::value) {
        return config.simDatarefsFile();
    }
    else if constexpr (std::is_same<md::CommandsFile, T>::value) {
        return config.simCommandsFile();
    }
    else {
        static_assert(false, "unknown type");
        return MaxSDK::Util::Path();
    }
}

template<typename T>
MaxSDK::Util::Path Dc<T>::projectDataFile(Config & config) {
    if constexpr (std::is_same<md::DatarefsFile, T>::value) {
        return config.projectDatarefsFile();
    }
    else if constexpr (std::is_same<md::CommandsFile, T>::value) {
        return config.projectCommandsFile();
    }
    else {
        static_assert(false, "unknown type");
        return MaxSDK::Util::Path();
    }
}

template<typename T>
void Dc<T>::loadSimDatarefs() {
    auto config = Config::instance();
    // MaxSDK::Util::Path::Exists have been added sine 3Ds Max 2013
    const auto pathConfMgr = IPathConfigMgr::GetPathConfigMgr();

    const auto datarefFile = simDataFile(*config);
    unloadIf([](const auto & v) { return !v->mIsForProject; });

    const auto simFolder = config->simDir();
    if (!pathConfMgr->DoesFileExist(simFolder)) {
        return;
    }

    if (!pathConfMgr->DoesFileExist(datarefFile)) {
        XLError << "X-Plane datarefs file isn't found by the path: " << xobj::fromMStr(datarefFile.GetString());
        return;
    }

    auto f = std::make_shared<IView::File>();
    if (f->loadSimData(datarefFile)) {
        f->sortDataIfEnabled();
        mDatarefs.emplace_back(std::move(f));
    }
}

template<typename T>
void Dc<T>::loadProjectDatarefs() {
    const auto config = Config::instance();
    // MaxSDK::Util::Path::Exists have been added sine 3Ds Max 2013
    const auto pathConfMgr = IPathConfigMgr::GetPathConfigMgr();

    const auto datarefFile = projectDataFile(*config);
    unloadIf([](const auto & v) { return v->mIsForProject; });

    if (!pathConfMgr->DoesFileExist(datarefFile)) {
        return;
    }

    auto f = std::make_shared<IView::File>();
    if (f->loadProjectData(datarefFile)) {
        f->sortDataIfEnabled();
        mDatarefs.emplace_back(std::move(f));
    }
}

template<typename T>
void Dc<T>::unloadIf(const std::function<bool(const typename IView::Files::value_type & v)> & fn) {
    for (auto iter = mDatarefs.begin(); iter != mDatarefs.end();) {
        if (fn(*iter)) {
            iter = mDatarefs.erase(iter);
        }
        else {
            ++iter;
        }
    }
}

/**************************************************************************************************/
//////////////////////////////////////////* Functions */////////////////////////////////////////////
/**************************************************************************************************/

inline void __readSettings(md::DatarefsFile::Ptr & file, Settings & settings) {
    if (file->mIsForProject) {
        file->mUsesId = settings.isUseDatarefsId();
        const auto sort = settings.sortDatarefs();
        if (file->mSort != sort) {
            file->mSort = sort;
            if (!file->mSort) {
                // reload from file
                try {
                    file->loadData(file->mFilePath);
                }
                catch (const std::exception & e) {
                    XLError << "Can't load file <" << xobj::fromMStr(file->mFilePath.GetString()) << "> reason: " << e.what();
                }
            }
            else {
                // sort
                file->sortDataIfEnabled();
            }
        }
    }
}

inline void __readSettings(md::CommandsFile::Ptr & file, Settings & settings) {
    if (file->mIsForProject) {
        file->mUsesId = settings.isUseCommandsId();
        const auto sort = settings.sortCommands();
        if (file->mSort != sort) {
            file->mSort = sort;
            if (!file->mSort) {
                // reload from file
                try {
                    file->loadData(file->mFilePath);
                }
                catch (const std::exception & e) {
                    XLError << "Can't load file <" << xobj::fromMStr(file->mFilePath.GetString()) << "> reason: " << e.what();
                }
            }
            else {
                // sort
                file->sortDataIfEnabled();
            }
        }
    }
}

template<typename T>
void Dc<T>::onSettingsChanged(Settings * settings) {
    for (auto & f : mDatarefs) {
        __readSettings(f, *settings);
    }
}

template<typename T>
void Dc<T>::onViewReady() {
    mView->setAvailableFiles(&mDatarefs);

    if (mDatarefs.empty()) {
        mSelectedFile = MaxSDK::Util::Path();
        mView->setCurrFile(nullptr, std::nullopt);
        mView->setCurrKey(mCurrKey);
        mView->setSearchKey(mCurrSearchKey);
        return;
    }

    if (!mCurrKey.isNull()) {
        for (const auto & file : mDatarefs) {
            std::optional<std::size_t> index = std::nullopt;
            if (std::isdigit(int(*mCurrKey.data()))) {
                try {
                    index = file->indexOfId(xobj::Dataref::keyToId(xobj::fromMStr(mCurrKey)));
                }
                catch (const std::exception & e) {
                    XLError << "Can't extract id from dataref. Reason: " << e.what();
                    break;
                }
            }
            else {
                index = file->indexOfKey(xobj::fromMStr(mCurrKey));
            }
            if (index) {
                mSelectedFile = file->mFilePath;
                mView->setCurrFile(file, index);
                mView->setSearchKey(mCurrSearchKey);
                return;
            }
        }
    }

    const auto index = indexOfPath(mDatarefs, mSelectedFile);
    const auto & file = mDatarefs.at(index.value_or(0));
    mSelectedFile = file->mFilePath;
    mView->setCurrFile(file, std::nullopt);
    mView->setCurrKey(mCurrKey);
    mView->setSearchKey(mCurrSearchKey);
}

template<typename T>
void Dc<T>::onKeyChanged(const typename IView::FilePtr & file, const MStr & key) {
    if (!file || !file->mUsesId) {
        mCurrKey = key;
        return;
    }

    // This algorithm set the correct id for id oriented data and
    // it saves data to the file but the file may have some changes
    // that has been made manually with a text editor or with something else like VCS.
    // So before save the file we try to reload it.
    try {
        file->loadData(file->mFilePath);
    }
    catch (const std::exception & e) {
        XLError << "Can't load file <" << xobj::fromMStr(file->mFilePath.GetString()) << "> reason: " << e.what();
    }

    const auto index = file->indexOfKey(xobj::fromMStr(key));
    if (!index) {
        mCurrKey = key;
        return;
    }
    auto & data = file->mData[*index];
    if (data.mId == xobj::Dataref::invalidId()) {
        data.mId = file->generateId();
        if (file->mIsEditable) {
            try {
                file->saveData(file->mFilePath);
            }
            catch (const std::exception & e) {
                XLError << "Can't save file <" << xobj::fromMStr(file->mFilePath.GetString()) << "> reason: " << e.what();
            }
        }
    }
    mCurrKey = xobj::toMStr(std::to_string(data.mId));
}

template<typename T>
void Dc<T>::onSearchKeyChanged(const MStr & data) {
    mCurrSearchKey = data;
}

template<typename T>
void Dc<T>::onSimDirChanged(Config &, const MaxSDK::Util::Path &, const MaxSDK::Util::Path &) {
    loadSimDatarefs();
}

template<typename T>
void Dc<T>::onCurrFileChanged(const MStr & name) {
    const auto index = indexOfDisplayName(mDatarefs, name);
    if (!index) {
        mView->setCurrFile(mDatarefs.at(0), std::nullopt);
        mView->setCurrKey(mCurrKey);
        mView->setSearchKey(mCurrSearchKey);
    }

    const auto & file = mDatarefs.at(*index);
    if (mSelectedFile == file->mFilePath) {
        return;
    }
    mSelectedFile = file->mFilePath;
    mView->setCurrFile(file, std::nullopt);
    mView->setCurrKey(mCurrKey);
    mView->setSearchKey(mCurrSearchKey);
}

template<typename T>
void Dc<T>::onFileOpened(void * param, NotifyInfo * info) {
    auto * d = static_cast<Dc<T>*>(param);
    d->onSystemNew(param, info);
}

template<typename T>
void Dc<T>::onSystemReset(void * param, NotifyInfo * info) {
    auto * d = static_cast<Dc<T>*>(param);
    d->onSystemNew(param, info);
}

template<typename T>
void Dc<T>::onSystemNew(void * param, NotifyInfo *) {
    auto * d = static_cast<Dc<T>*>(param);
    d->loadProjectDatarefs();
}

/**************************************************************************************************/
//////////////////////////////////////////* Functions */////////////////////////////////////////////
/**************************************************************************************************/

template<typename T>
std::optional<std::size_t> Dc<T>::indexOfPath(const typename IView::Files & data, const MaxSDK::Util::Path & path) {
    std::size_t counter = 0;
    for (const auto & f : data) {
        if (f->mFilePath == path) {
            return counter;
        }
        ++counter;
    }
    return std::nullopt;
}

template<typename T>
std::optional<std::size_t> Dc<T>::indexOfDisplayName(const typename IView::Files & data, const MStr & displayName) {
    std::size_t counter = 0;
    for (const auto & f : data) {
        if (f->mDisplayName == displayName) {
            return counter;
        }
        ++counter;
    }
    return std::nullopt;
}

/**************************************************************************************************/
////////////////////////////////////////////////////////////////////////////////////////////////////
/**************************************************************************************************/
}
