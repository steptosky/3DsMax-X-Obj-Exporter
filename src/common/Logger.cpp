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

#include "Logger.h"
#include "Info.h"

#pragma warning(push, 0)
#include <max.h>
#include <log.h>
#include <IPathConfigMgr.h>
#pragma warning(pop)

#include "common/String.h"
#include <xpln/common/ExternalLog.h>

#ifndef NDEBUG
#	define LOGLEVEL sts::BaseLogger::eType::Debug
#else
#	define LOGLEVEL sts::BaseLogger::eType::Msg
#endif

#define LOGPREFIX "[X-Obj]"

/**************************************************************************************************/
//////////////////////////////////////////* Static area *///////////////////////////////////////////
/**************************************************************************************************/

sts::BaseLogger sts::BaseLogger::mInstance;
std::list<Logger::UserConsoleCallBack> Logger::mCallbacks;
LogSys * Logger::mMaxLog = nullptr;

/**************************************************************************************************/
//////////////////////////////////////////* Static area *///////////////////////////////////////////
/**************************************************************************************************/

void xLibLogCallBack(xobj::ExternalLog::eType inType, const char * inMsg,
					const char * file, int line, const char * function, const char * category) {
	Logger::logCallBack(static_cast<sts::BaseLogger::eType>(inType), inMsg, file, line, function, category);
}

/**************************************************************************************************/
//////////////////////////////////////////* Static area *///////////////////////////////////////////
/**************************************************************************************************/

/*!
 * \note Example path to log file "C:\Users\Alex\AppData\Local\Autodesk\3dsMax\2016 - 64bit\ENU\Network\Max.log"
 */
void Logger::logCallBack(sts::BaseLogger::eType inType, const char * inMsg,
						const char * inFile, int inLine, const char * inFunction, const char * inCategory) {

	if (inType > LOGLEVEL) {
		return;
	}

	DWORD msgType = SYSLOG_INFO;
	BOOL dialog = NO_DIALOG;
	if (inCategory) {
		if (strcmp(inCategory, LOG_CATEGORY_DIALOG) == 0) {
			dialog = DISPLAY_DIALOG;
		}
		else if (strcmp(inCategory, LOG_CATEGORY_CONSOLE) == 0 || strcmp(inCategory, LOG_CATEGORY_FOR_USER) == 0) {
			for (auto c : mCallbacks) {
				c(inType, inMsg);
			}
		}
	}
	if (inType == sts::BaseLogger::eType::Warning) {
		msgType = SYSLOG_WARN;
	}
	else if (inType == sts::BaseLogger::eType::Error) {
		msgType = SYSLOG_ERROR;
	}
	else if (inType == sts::BaseLogger::eType::Critical) {
		msgType = SYSLOG_ERROR;
	}
	else if (inType == sts::BaseLogger::eType::Fatal) {
		msgType = SYSLOG_ERROR;
		dialog = DISPLAY_DIALOG;
	}
	else if (inType == sts::BaseLogger::eType::Debug) {
		msgType = SYSLOG_DEBUG;
	}

	if (msgType == SYSLOG_INFO) {
		mMaxLog->LogEntry(msgType, dialog, _M(XIO_PROJECT_SHORT_NAME), inCategory ? _M("%s %s %s") : _M("%s%s %s"),
						_M(LOGPREFIX), inCategory ? sts::toString(inCategory).c_str() : _M(""), sts::toString(inMsg).c_str());
		Debug(std::cout << sts::BaseLogger::typeAsString(inType) << ": "
			<< (inCategory ? inCategory : "") << (inCategory ? " " : "") << (inMsg ? inMsg : "") << std::endl);
	}
	else {
		mMaxLog->LogEntry(msgType, dialog, _M(XIO_PROJECT_SHORT_NAME), inCategory ? _M("%s %s %s") : _M("%s%s %s"),
						_M(LOGPREFIX), inCategory ? sts::toString(inCategory).c_str() : _M(""), sts::toString(inMsg).c_str());
		mMaxLog->LogEntry(msgType, NO_DIALOG, _M(XIO_PROJECT_SHORT_NAME), _M("%s\t\t<%s -> %s(%d)>"),
						_M(LOGPREFIX), sts::toString(inFunction).c_str(), sts::toString(inFile).c_str(), inLine);
		Debug(std::cout << sts::BaseLogger::typeAsString(inType) << ": "
			<< (inCategory ? inCategory : "") << (inCategory ? " " : "") << (inMsg ? inMsg : "") << LEOL <<"\t<"
			<< (inFunction ? inFunction : "") << " -> " << (inFile ? inFile : "")
			<< "(" << inLine << ")>" << std::endl);
	}
}

/**************************************************************************************************/
//////////////////////////////////////////* Functions */////////////////////////////////////////////
/**************************************************************************************************/

void Logger::saveLog(const MSTR & where) {
	IPathConfigMgr * paths = IPathConfigMgr::GetPathConfigMgr();
	MaxSDK::Util::Path p(paths->GetDir(APP_MAXDATA_DIR));
	p.AddTrailingBackslash();
	p.Append(_T("Network"));
	p.AddTrailingBackslash();
	p.Append(_T("Max.Log"));
	if (!paths->DoesFileExist(p)) {
		MessageBoxA(GetActiveWindow(),
					"The log file does not exist. For some reason the 3DsMax did not provide this file.\r\nCheck 3DsMax log settings.",
					"Error", MB_ICONERROR);
	}
	else {
		if (!CopyFile(p.GetCStr(), where.data(),FALSE)) {
			DWORD err = GetLastError();
			MessageBoxA(GetActiveWindow(), std::string("Can't save log file. code:").append(sts::toMbString(uint64_t(err))).c_str(),
						"Error", MB_ICONERROR);
		}
	}
}

/**************************************************************************************************/
////////////////////////////////////* Constructors/Destructor */////////////////////////////////////
/**************************************************************************************************/

Logger::Logger() {
	if (!mMaxLog) {
		mMaxLog = GetCOREInterface()->Log();
	}

	sts::BaseLogger & log = sts::BaseLogger::instance();
	log.setCallBack(&Logger::logCallBack);
	//xobj::ExternalLog::registerCallBack(&xLibLogCallBack);
	log.setLevel(sts::BaseLogger::eType::Debug);
	printInformation();
	log.setLevel(LOGLEVEL);
}

Logger::~Logger() {
	xobj::ExternalLog::unRegisterCallBack();
}

/**************************************************************************************************/
///////////////////////////////////////////* Functions *////////////////////////////////////////////
/**************************************************************************************************/

std::string Logger::aboutXLibInfo(bool inUseWinEol) {
	return xobj::ExternalLog::about(inUseWinEol);
}

std::string Logger::shortAboutXLibInfo(bool inUseWinEol) {
	return xobj::ExternalLog::shortAbout(inUseWinEol);
}

std::string Logger::aboutInfo(bool inUseWinEol) {
	std::stringstream stream;
	const char * eol = inUseWinEol ? "\r\n" : "\n";
	//-------------------------------------------------------------------------

#ifndef MAX_PRODUCT_YEAR_NUMBER
	stream << "Project: " XIO_PROJECT_NAME << " " << MAX_VERSION_MAJOR << eol;
#else
	stream << "Project: " XIO_PROJECT_NAME << " " << MAX_PRODUCT_YEAR_NUMBER << " (" << MAX_VERSION_MAJOR << ")" << eol;
#endif

	stream << "Organization: " << XIO_ORGANIZATION_NAME << " (" << XIO_ORGANIZATION_WEBLINK << ")" << eol;
	stream << "Desc: " << XIO_PROJECT_DESCRIPTION << eol;
	stream << "Link: " << XIO_PROJECT_WEBLINK << eol;

	stream << "Version: " XIO_VERSION_STRING << "-" << XIO_RELEASE_TYPE << "+" << XIO_VCS_REVISION << " (" << XIO_VCS_BRANCH << ") "
			<< XIO_COMPILE_DATE << Debug(" (" << XIO_COMPILE_TIME << ") " << "DEBUG" << ) eol;

	stream << "Compiler: " << XIO_COMPILER_NAME << " " << XIO_COMPILER_VERSION << eol;
	stream << XIO_COPYRIGHT << eol;
	stream << "Contacts: " << XIO_ORGANIZATION_WEBLINK << eol;
	stream << "License: " << XIO_LICENSE_TYPE << eol;
	stream << "Sources: " << XIO_PROJECT_SOURCES_WEBLINK << eol;

	stream << "Contributors: " << eol;
	for (size_t i = 0; i < XIO_ARRAY_LENGTH(XIO_CONTRIBUTORS); ++i) {
		stream << "    " << XIO_CONTRIBUTORS[i] << eol;
	}

	//-------------------------------------------------------------------------

	return stream.str();
}

std::string Logger::shortAboutInfo(bool inUseWinEol) {
	std::stringstream stream;
	const char * eol = inUseWinEol ? "\r\n" : "\n";
	//-------------------------------------------------------------------------

#ifndef MAX_PRODUCT_YEAR_NUMBER
	stream << "Project: " XIO_PROJECT_NAME << " " << MAX_VERSION_MAJOR << eol;
#else
	stream << "Project: " XIO_PROJECT_NAME << " " << MAX_PRODUCT_YEAR_NUMBER << " (" << MAX_VERSION_MAJOR << ")" << eol;
#endif
	stream << "Version: " XIO_VERSION_STRING << "-" << XIO_RELEASE_TYPE << "+" << XIO_VCS_REVISION << " (" << XIO_VCS_BRANCH << ") "
			<< XIO_COMPILE_DATE << Debug(" (" << XIO_COMPILE_TIME << ") " << "DEBUG" << ) eol;

	stream << "Compiler: " << XIO_COMPILER_NAME << " " << XIO_COMPILER_VERSION << eol;

	//-------------------------------------------------------------------------

	return stream.str();
}

void Logger::printInformation() {
	auto msg1 = sts::StringUtils<char>::split<std::list<std::string>>(shortAboutInfo(false), "\n");
	auto msg2 = sts::StringUtils<char>::split<std::list<std::string>>(shortAboutXLibInfo(false), "\n");
	LMessage << "************************************************************";
	for (auto s : msg1) {
		LMessage << s;
	}
	LMessage << "************************************************************";
	for (auto s : msg2) {
		LMessage << s;
	}
	LMessage << "************************************************************";
}

/**************************************************************************************************/
///////////////////////////////////////////* Functions *////////////////////////////////////////////
/**************************************************************************************************/

void Logger::registerUserConsoleCallback(UserConsoleCallBack inCallback) {
	mCallbacks.emplace_back(inCallback);
}

void Logger::unregisterUserConsoleCallback(UserConsoleCallBack inCallback) {
	for (auto it = mCallbacks.begin(); it != mCallbacks.end(); ++it) {
		if (*it == inCallback) {
			mCallbacks.erase(it);
			return;
		}
	}
}

/**************************************************************************************************/
////////////////////////////////////////////////////////////////////////////////////////////////////
/**************************************************************************************************/
