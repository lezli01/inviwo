/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2024 Inviwo Foundation
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

#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/processors/processorwidgetfactory.h>
#include <inviwo/core/metadata/processormetadata.h>
#include <inviwo/core/util/vectoroperations.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/raiiutils.h>
#include <inviwo/core/util/rendercontext.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/util/utilities.h>
#include <inviwo/core/network/processornetworkconverter.h>
#include <inviwo/core/network/networklock.h>
#include <inviwo/core/metadata/processormetadata.h>
#include <inviwo/core/network/networkvisitor.h>
#include <inviwo/core/network/networkedge.h>

#include <fmt/format.h>
#include <fmt/std.h>

#include <algorithm>

namespace inviwo {

ProcessorNetwork::ProcessorNetwork(InviwoApplication* application)
    : ProcessorNetworkObservable()
    , ProcessorObserver()
    , PropertyOwnerObserver()
    , application_(application)
    , linkEvaluator_(this) {}

ProcessorNetwork::~ProcessorNetwork() {
    lock();
    clear();
}

Processor* ProcessorNetwork::addProcessor(std::shared_ptr<Processor> processor) {
    NetworkLock lock(this);

    processor->setIdentifier(util::findUniqueIdentifier(
        util::stripIdentifier(processor->getIdentifier()),
        [&](std::string_view id) { return getProcessorByIdentifier(id) == nullptr; }, ""));

    notifyObserversProcessorNetworkWillAddProcessor(processor.get());
    processors_[processor->getIdentifier()] = processor;
    processor->setNetwork(this);
    processor->ProcessorObservable::addObserver(this);
    onIdChange_[processor.get()] =
        processor->onIdentifierChange([this](std::string_view newID, std::string_view oldID) {
            const std::string old{oldID};
            processors_[std::string{newID}] = processors_[old];
            processors_.erase(old);
        });
    addPropertyOwnerObservation(processor.get());

    auto* meta = processor->getMetaData<ProcessorMetaData>(ProcessorMetaData::CLASS_IDENTIFIER);
    meta->addObserver(this);

    if (application_) {
        if (auto widget = application_->getProcessorWidgetFactory()->create(processor.get())) {
            processor->setProcessorWidget(std::move(widget));
        }
    }

    processor->invalidate(InvalidationLevel::InvalidResources);
    notifyObserversProcessorNetworkDidAddProcessor(processor.get());
    return processor.get();
}

void ProcessorNetwork::removeProcessorHelper(Processor* processor) {
    // Remove all connections for this processor
    for (auto* outport : processor->getOutports()) {
        const std::vector<Inport*> inports = outport->getConnectedInports();
        for (auto* inport : inports) {
            removeConnection(outport, inport);
        }
    }
    for (auto* inport : processor->getInports()) {
        const std::vector<Outport*> outports = inport->getConnectedOutports();
        for (auto* outport : outports) {
            removeConnection(outport, inport);
        }
    }

    // Remove all links for this processor
    auto toDelete =
        util::copy_if(links_, [&](const PropertyLink& link) { return link.involves(processor); });
    for (auto& link : toDelete) {
        removeLink(link.getSource(), link.getDestination());
    }
}

std::shared_ptr<Processor> ProcessorNetwork::removeProcessor(std::string_view identifier) {
    return removeProcessor(getProcessorByIdentifier(identifier));
}

std::shared_ptr<Processor> ProcessorNetwork::removeProcessor(Processor* processor) {
    if (!processor) return nullptr;

    auto it = processors_.find(processor->getIdentifier());
    if (it == processors_.end()) return nullptr;

    auto owning = it->second;

    const NetworkLock lock(this);

    rendercontext::activateDefault();
    removeProcessorHelper(processor);

    // remove processor itself
    notifyObserversProcessorNetworkWillRemoveProcessor(processor);
    processors_.erase(it);
    processor->ProcessorObservable::removeObserver(this);
    onIdChange_.erase(processor);
    removePropertyOwnerObservation(processor);
    processor->setNetwork(nullptr);
    processor->setProcessorWidget(nullptr);
    notifyObserversProcessorNetworkDidRemoveProcessor(processor);

    return owning;
}

Processor* ProcessorNetwork::getProcessorByIdentifier(std::string_view identifier) const {
    return util::map_find_or_null(processors_, std::string(identifier),
                                  [](const auto& p) { return p.get(); });
}

std::vector<Processor*> ProcessorNetwork::getProcessors() const {
    return util::transform(processors_, [](const auto& elem) { return elem.second.get(); });
}

void ProcessorNetwork::addConnection(const PortConnection& connection) {
    addConnection(connection.getOutport(), connection.getInport());
}
void ProcessorNetwork::addConnection(Outport* src, Inport* dst) {
    if (!isPortInNetwork(src)) {
        throw Exception(
            fmt::format(
                "Unable to create connection, Outport '{}' of Processor '{}' not found in network",
                src->getClassIdentifier(), src->getProcessor()->getIdentifier()),
            IVW_CONTEXT);
    }
    if (!isPortInNetwork(dst)) {
        throw Exception(
            fmt::format(
                "Unable to create connection, Inport '{}' of Processor '{}' not found in network",
                src->getClassIdentifier(), src->getProcessor()->getIdentifier()),
            IVW_CONTEXT);
    }

    if (canConnect(src, dst) && !isConnected(src, dst)) {
        const NetworkLock lock(this);

        const PortConnection connection(src, dst);
        notifyObserversProcessorNetworkWillAddConnection(connection);

        dst->connectTo(src);
        connections_.emplace(src, dst);
        connectionsVec_.emplace_back(src, dst);

        notifyObserversProcessorNetworkDidAddConnection(connection);
    }
}

bool ProcessorNetwork::canConnect(const Outport* src, const Inport* dst) const {
    return src != nullptr && dst != nullptr && dst->canConnectTo(src);
}

void ProcessorNetwork::removeConnection(const PortConnection& connection) {
    auto it = connections_.find(connection);
    if (it != connections_.end()) {
        const NetworkLock lock(this);

        notifyObserversProcessorNetworkWillRemoveConnection(connection);

        connection.getInport()->disconnectFrom(connection.getOutport());
        connections_.erase(it);
        std::erase(connectionsVec_, connection);

        notifyObserversProcessorNetworkDidRemoveConnection(connection);
    }
}
void ProcessorNetwork::removeConnection(Outport* src, Inport* dst) {
    const PortConnection connection(src, dst);
    removeConnection(connection);
}

bool ProcessorNetwork::isConnected(const PortConnection& connection) const {
    return connections_.find(connection) != connections_.end();
}
bool ProcessorNetwork::isConnected(Outport* src, Inport* dst) const {
    return isConnected(PortConnection(src, dst));
}

const std::vector<PortConnection>& ProcessorNetwork::getConnections() const {
    return connectionsVec_;
}

bool ProcessorNetwork::isPortInNetwork(Port* port) const {
    if (auto* processor = port->getProcessor()) {
        if (processor == getProcessorByIdentifier(processor->getIdentifier())) {
            return true;
        }
    }
    return false;
}

void ProcessorNetwork::addLink(const PropertyLink& link) {
    addLink(link.getSource(), link.getDestination());
}
void ProcessorNetwork::addLink(Property* src, Property* dst) {
    if (!src) {
        throw Exception("Source property is a nullptr", IVW_CONTEXT);
    }
    if (!dst) {
        throw Exception("Destination property is a nullptr", IVW_CONTEXT);
    }

    if (!isPropertyInNetwork(src)) {
        throw Exception("Source property not found in network", IVW_CONTEXT);
    }
    if (!isPropertyInNetwork(dst)) {
        throw Exception("Destination property not found in network", IVW_CONTEXT);
    }

    if (!isLinked(src, dst) && canLink(src, dst)) {
        const NetworkLock lock(this);
        const PropertyLink link(src, dst);
        notifyObserversProcessorNetworkWillAddLink(link);
        links_.insert(link);
        linkEvaluator_.addLink(link);  // add to cache
        notifyObserversProcessorNetworkDidAddLink(link);
    }
}

bool ProcessorNetwork::canLink(const Property* src, const Property* dst) const {
    return linkEvaluator_.canLink(src, dst);
}

void ProcessorNetwork::removeLink(const PropertyLink& link) {
    auto it = links_.find(link);
    if (it != links_.end()) {
        const NetworkLock lock(this);
        notifyObserversProcessorNetworkWillRemoveLink(link);
        linkEvaluator_.removeLink(link);
        links_.erase(it);
        notifyObserversProcessorNetworkDidRemoveLink(link);
    }
}
void ProcessorNetwork::removeLink(Property* src, Property* dst) {
    const PropertyLink link(src, dst);
    removeLink(link);
}

void ProcessorNetwork::onWillRemoveProperty(Property* property, size_t /*index*/) {
    if (auto* comp = dynamic_cast<PropertyOwner*>(property)) {
        size_t i = 0;
        for (auto* p : comp->getProperties()) {
            onWillRemoveProperty(p, i);
            i++;
        }
    }

    auto toDelete =
        util::copy_if(links_, [&](const PropertyLink& link) { return link.involves(property); });
    for (auto& link : toDelete) removeLink(link);
}

bool ProcessorNetwork::isLinked(const PropertyLink& link) const {
    return links_.find(link) != links_.end();
}
bool ProcessorNetwork::isLinked(Property* src, Property* dst) const {
    return isLinked(PropertyLink(src, dst));
}

std::vector<PropertyLink> ProcessorNetwork::getLinks() const {
    return util::transform(links_, [](PropertyLink elem) { return elem; });
}

bool ProcessorNetwork::isLinkedBidirectional(Property* src, Property* dst) const {
    return isLinked(src, dst) && isLinked(dst, src);
}

std::vector<Property*> ProcessorNetwork::getPropertiesLinkedTo(Property* property) {
    return linkEvaluator_.getPropertiesLinkedTo(property);
}

std::vector<PropertyLink> ProcessorNetwork::getLinksBetweenProcessors(Processor* p1,
                                                                      Processor* p2) {
    return linkEvaluator_.getLinksBetweenProcessors(p1, p2);
}

void ProcessorNetwork::evaluateLinksFromProperty(Property* source) {
    linkEvaluator_.evaluateLinksFromProperty(source);
}

void ProcessorNetwork::clear() {
    const NetworkLock lock(this);

    auto processors = getProcessors();
    for (auto* processor : processors) {
        removeProcessor(processor);
    }
}

bool ProcessorNetwork::empty() const { return processors_.empty(); }

size_t ProcessorNetwork::size() const { return processors_.size(); }

void ProcessorNetwork::accept(NetworkVisitor& visitor) {
    for (auto& p : processors_) {
        p.second->accept(visitor);
    }
}

bool ProcessorNetwork::isEmpty() const { return processors_.empty(); }

bool ProcessorNetwork::isInvalidating() const { return !processorsInvalidating_.empty(); }

bool ProcessorNetwork::isLinking() const { return linkEvaluator_.isLinking(); }

void ProcessorNetwork::onProcessorInvalidationBegin(Processor* p) {
    util::push_back_unique(processorsInvalidating_, p);
}

void ProcessorNetwork::onProcessorInvalidationEnd(Processor* p) {
    std::erase(processorsInvalidating_, p);

    if (processorsInvalidating_.empty()) {
        notifyObserversProcessorNetworkEvaluateRequest();
    }
}

void ProcessorNetwork::onProcessorPortRemoved(Processor*, Port* port) {
    auto toDelete = util::copy_if(connectionsVec_, [&](const PortConnection& item) {
        return item.getInport() == port || item.getOutport() == port;
    });
    for (auto& item : toDelete) {
        removeConnection(item.getOutport(), item.getInport());
    }
}

void ProcessorNetwork::onProcessorStartBackgroundWork(Processor* p, size_t jobs) {
    backgoundJobs_ += static_cast<int>(jobs);
    notifyObserversProcessorBackgroundJobsChanged(p, static_cast<int>(jobs), backgoundJobs_);
}

void ProcessorNetwork::onProcessorFinishBackgroundWork(Processor* p, size_t jobs) {
    backgoundJobs_ -= static_cast<int>(jobs);
    notifyObserversProcessorBackgroundJobsChanged(p, -static_cast<int>(jobs), backgoundJobs_);
}

void ProcessorNetwork::onAboutPropertyChange(Property* modifiedProperty) {
    if (modifiedProperty) linkEvaluator_.evaluateLinksFromProperty(modifiedProperty);
    notifyObserversProcessorNetworkChanged();
}

void ProcessorNetwork::onProcessorMetaDataPositionChange() {
    notifyObserversProcessorNetworkChanged();
}
void ProcessorNetwork::onProcessorMetaDataVisibilityChange() {
    notifyObserversProcessorNetworkChanged();
}
void ProcessorNetwork::onProcessorMetaDataSelectionChange() {
    notifyObserversProcessorNetworkChanged();
}

void ProcessorNetwork::serialize(Serializer& s) const {
    s.serialize("ProcessorNetworkVersion", processorNetworkVersion_);
    s.serialize("Processors", getProcessors(), "Processor");

    {
        std::vector<NetworkEdge> connections;
        connections.reserve(connectionsVec_.size());
        for (const auto& item : connectionsVec_) {
            connections.emplace_back(item);
        }
        s.serialize("Connections", connections, "Connection");
    }

    {
        std::vector<NetworkEdge> links;
        links.reserve(links_.size());
        for (const auto& item : links_) {
            links.emplace_back(item);
        }
        s.serialize("PropertyLinks", links, "PropertyLink");
    }
}

void ProcessorNetwork::addPropertyOwnerObservation(PropertyOwner* po) {
    po->addObserver(this);
    for (auto* child : po->getCompositeProperties()) {
        addPropertyOwnerObservation(child);
    }
}

void ProcessorNetwork::removePropertyOwnerObservation(PropertyOwner* po) {
    po->removeObserver(this);
    for (auto* child : po->getCompositeProperties()) {
        removePropertyOwnerObservation(child);
    }
}

int ProcessorNetwork::getVersion() { return processorNetworkVersion_; }

const int ProcessorNetwork::processorNetworkVersion_ = 21;

void ProcessorNetwork::deserialize(Deserializer& d) {
    const NetworkLock lock(this);

    // This will set deserializing_ to true while keepTrueWillAlive is in scope
    // and set it to false no matter how we leave the scope
    const util::KeepTrueWhileInScope keepTrueWillAlive(&deserializing_);

    int version = 0;
    d.deserialize("ProcessorNetworkVersion", version);

    if (version != processorNetworkVersion_) {
        LogNetworkSpecial((&d), LogLevel::Warn,
                          fmt::format("Loading old workspace ({}) Processor Network version: {}. "
                                      "Updating to version: {}.",
                                      d.getFileName(), version, processorNetworkVersion_));
        ProcessorNetworkConverter nv(version);
        d.convertVersion(&nv);
    }

    // Processors
    try {
        rendercontext::activateDefault();

        auto des = util::MapDeserializer<std::string, std::shared_ptr<Processor>>(
                       "Processors", "Processor", "identifier")
                       .setIdentifierTransform(
                           [](const std::string& id) { return util::stripIdentifier(id); })
                       .setMakeNew([]() {
                           rendercontext::activateDefault();
                           return nullptr;
                       })
                       .onNew([&](const std::string& /*id*/, std::shared_ptr<Processor>& p) {
                           addProcessor(p);
                       })
                       .onRemove([&](const std::string& id) { removeProcessor(id); });
        des(d, processors_);

    } catch (const Exception& exception) {
        clear();
        throw AbortException("Deserialization error: " + exception.getMessage(),
                             exception.getContext());
    } catch (const std::exception& exception) {
        clear();
        throw AbortException("Deserialization error: " + std::string(exception.what()),
                             IVW_CONTEXT);
    } catch (...) {
        clear();
        throw AbortException("Unknown Exception during deserialization.", IVW_CONTEXT);
    }

    // Connections
    try {
        std::vector<NetworkEdge> connectionsEdges;
        d.deserialize("Connections", connectionsEdges, "Connection");

        std::vector<PortConnection> connections;
        for (const auto& edge : connectionsEdges) {
            try {
                connections.emplace_back(edge.toConnection(*this));
            } catch (...) {
                d.handleError(IVW_CONTEXT);
            }
        }

        // remove any already existing connections.
        std::unordered_set<PortConnection> save;
        std::erase_if(connections, [&](const auto& c) {
            if (connections_.count(c) != 0) {
                save.insert(c);
                return true;
            } else {
                return false;
            }
        });

        // remove any no longer used connections
        auto remove = util::copy_if(connections_, [&](auto& c) { return save.count(c) == 0; });
        for (auto& c : remove) removeConnection(c);

        // Add the new connections
        for (auto& c : connections) {
            try {
                addConnection(c);
            } catch (...) {
                d.handleError(IVW_CONTEXT);
            }
        }

    } catch (const Exception& exception) {
        clear();
        throw IgnoreException("Deserialization error: " + exception.getMessage(),
                              exception.getContext());
    } catch (const std::exception& exception) {
        clear();
        throw AbortException("Deserialization error: " + std::string(exception.what()),
                             IVW_CONTEXT);
    } catch (...) {
        clear();
        throw AbortException("Unknown Exception during deserialization.", IVW_CONTEXT);
    }

    // Links
    try {
        std::vector<NetworkEdge> linkEdges;
        d.deserialize("PropertyLinks", linkEdges, "PropertyLink");

        std::vector<PropertyLink> links;
        for (const auto& item : linkEdges) {
            try {
                links.emplace_back(item.toLink(*this));
            } catch (...) {
                d.handleError(IVW_CONTEXT);
            }
        }

        // remove any already existing links.
        std::unordered_set<PropertyLink> save;
        std::erase_if(links, [&](const auto& l) {
            if (links_.count(l) != 0) {
                save.insert(l);
                return true;
            } else {
                return false;
            }
        });

        // remove any no longer used links
        auto remove = util::copy_if(links_, [&](auto& l) { return save.count(l) == 0; });
        for (auto& l : remove) removeLink(l);

        // Add the new links
        for (auto& link : links) {
            try {
                addLink(link);
            } catch (...) {
                d.handleError(IVW_CONTEXT);
            }
        }

    } catch (const Exception& exception) {
        clear();
        throw IgnoreException("Deserialization error: " + exception.getMessage(),
                              exception.getContext());
    } catch (const std::exception& exception) {
        clear();
        throw AbortException("Deserialization error: " + std::string(exception.what()),
                             IVW_CONTEXT);
    } catch (...) {
        clear();
        throw AbortException("Unknown Exception during deserialization.", IVW_CONTEXT);
    }

    notifyObserversProcessorNetworkChanged();
}

bool ProcessorNetwork::isDeserializing() const { return deserializing_; }

Property* ProcessorNetwork::getProperty(std::string_view path) const {
    const auto [processorId, propertyPath] = util::splitByFirst(path, '.');
    if (auto* processor = getProcessorByIdentifier(processorId)) {
        return processor->getPropertyByPath(propertyPath);
    }
    return nullptr;
}

Port* ProcessorNetwork::getPort(std::string_view path) const {
    const auto [processorId, portId] = util::splitByFirst(path, '.');
    if (auto* processor = getProcessorByIdentifier(processorId)) {
        return processor->getPort(portId);
    }
    return nullptr;
}

Inport* ProcessorNetwork::getInport(std::string_view path) const {
    const auto [processorId, portId] = util::splitByFirst(path, '.');
    if (auto* processor = getProcessorByIdentifier(processorId)) {
        return processor->getInport(portId);
    }
    return nullptr;
}

Outport* ProcessorNetwork::getOutport(std::string_view path) const {
    const auto [processorId, portId] = util::splitByFirst(path, '.');
    if (auto* processor = getProcessorByIdentifier(processorId)) {
        return processor->getOutport(portId);
    }
    return nullptr;
}

bool ProcessorNetwork::isPropertyInNetwork(Property* prop) const {
    if (auto* owner = prop->getOwner()) {
        if (auto* processor = owner->getProcessor()) {
            return processor == util::map_find_or_null(processors_, processor->getIdentifier(),
                                                       [](const auto& p) { return p.get(); });
        }
    }
    return false;
}

InviwoApplication* ProcessorNetwork::getApplication() const { return application_; }

void ProcessorNetwork::assignIdentifierAndName(Processor& processor, std::string_view name) {
    if (processor.getIdentifier().empty()) {
        processor.setIdentifier(util::stripIdentifier(name));
    }
    if (processor.getDisplayName().empty()) {
        processor.setDisplayName(name);
    }
}

}  // namespace inviwo
