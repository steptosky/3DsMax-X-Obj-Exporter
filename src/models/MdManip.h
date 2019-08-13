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
#pragma warning(pop)

#include "models/io/ManipIO.h"

/********************************************************************************************************/
//////////////////////////////////////////////////////////////////////////////////////////////////////////
/********************************************************************************************************/

class MdManip {
public:

    MdManip();
    ~MdManip();

    //-------------------------------------------------------------------------

    void reset();
    static void cloneData(INode * from, INode * to);

    //-------------------------------------------------------------------------

    void saveToNode(const std::optional<xobj::AttrManip> & inManip) const { saveToNode(mNode, inManip); }
    bool loadFromNode(ManipIO::IManipIo * callBack) { return loadFromNode(mNode, callBack); }

    //-------------------------------------------------------------------------

    bool hasLink() const { return mNode != nullptr; }
    void clearLink();

    bool linkNode(INode * node);
    const INode * linkedNode() const { return mNode; }
    INode * linkedNode() { return mNode; }

    //-------------------------------------------------------------------------

private:

    //-------------------------------------------------------------------------

    void saveToNode(INode * node, const std::optional<xobj::AttrManip> & inManip) const;
    bool loadFromNode(INode * node, ManipIO::IManipIo * callBack);

    INode * mNode = nullptr;

    //-------------------------------------------------------------------------
};

/********************************************************************************************************/
//////////////////////////////////////////////////////////////////////////////////////////////////////////
/********************************************************************************************************/
