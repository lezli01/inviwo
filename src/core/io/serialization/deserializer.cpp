/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2023 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/

#include <inviwo/core/io/serialization/deserializer.h>
#include <inviwo/core/io/serialization/serializable.h>
#include <inviwo/core/io/serialization/versionconverter.h>
#include <inviwo/core/util/factory.h>
#include <inviwo/core/util/exception.h>

#include <inviwo/core/io/serialization/ticpp.h>

namespace inviwo {

Deserializer::Deserializer(const std::filesystem::path& fileName) : SerializeBase(fileName) {
    try {
        doc_->LoadFile(TIXML_ENCODING_UTF8);
        rootElement_ = doc_->FirstChildElement();
        rootElement_->GetAttribute(std::string{SerializeConstants::VersionAttribute},
                                   &inviwoWorkspaceVersion_, false);
    } catch (TxException& e) {
        throw AbortException(e.what(), IVW_CONTEXT);
    }
}

Deserializer::Deserializer(std::istream& stream, const std::filesystem::path& path)
    : SerializeBase(stream, path) {
    try {
        // Base streamed in the xml data. Get the first node.
        rootElement_ = doc_->FirstChildElement();
    } catch (TxException& e) {
        throw AbortException(e.what(), IVW_CONTEXT);
    }
}

void Deserializer::deserialize(std::string_view key, std::filesystem::path& path,
                               const SerializationTarget& target) {

    try {
        if (target == SerializationTarget::Attribute) {
            const auto& val = rootElement_->GetAttribute(key);
            if (!val.empty()) {
                path = val;
            }
        } else {
            if (NodeSwitch ns{*this, key}) {
                const auto& val = rootElement_->GetAttribute(SerializeConstants::ContentAttribute);
                if (!val.empty()) {
                    path = val;
                }
                return;
            }
            if (NodeSwitch ns{*this, key, true}) {
                const auto& val = rootElement_->GetAttribute(SerializeConstants::ContentAttribute);
                if (!val.empty()) {
                    path = val;
                }
                return;
            }
        }
    } catch (...) {
        handleError(IVW_CONTEXT);
    }
}

void Deserializer::deserialize(std::string_view key, Serializable& sObj) {
    if (NodeSwitch ns{*this, key}) sObj.deserialize(*this);
}

void Deserializer::deserialize(std::string_view key, signed char& data,
                               const SerializationTarget& target) {
    int val = data;
    deserialize(key, val, target);
    data = static_cast<char>(val);
}
void Deserializer::deserialize(std::string_view key, char& data,
                               const SerializationTarget& target) {
    int val = data;
    deserialize(key, val, target);
    data = static_cast<char>(val);
}
void Deserializer::deserialize(std::string_view key, unsigned char& data,
                               const SerializationTarget& target) {
    unsigned int val = data;
    deserialize(key, val, target);
    data = static_cast<unsigned char>(val);
}

void Deserializer::setExceptionHandler(ExceptionHandler handler) { exceptionHandler_ = handler; }

void Deserializer::convertVersion(VersionConverter* converter) { converter->convert(rootElement_); }

void Deserializer::handleError(const ExceptionContext& context) {
    if (exceptionHandler_) {
        exceptionHandler_(context);
    } else {  // If no error handler found:
        try {
            throw;
        } catch (SerializationException& e) {
            util::log(getLogger(), e.getContext(), e.getMessage(), LogLevel::Warn);
        }
    }
}

TxElement* Deserializer::retrieveChild(std::string_view key) {
    return retrieveChild_ ? rootElement_->FirstChildElement(key, false) : rootElement_;
}

void Deserializer::registerFactory(FactoryBase* factory) {
    registeredFactories_.push_back(factory);
}

int Deserializer::getInviwoWorkspaceVersion() const { return inviwoWorkspaceVersion_; }

const std::string& detail::getAttribute(TxElement* node, std::string_view key) {
    return node->GetAttribute(key);
}

void detail::forEachChild(TxElement* node, std::string_view key,
                          std::function<void(TxElement*)> func) {

    TxEIt child{key};

    for (child = child.begin(node); child != child.end(); ++child) {
        func(&(*child));
    }
}

}  // namespace inviwo
