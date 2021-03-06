//
//  equiv_address.cpp
//  blocksci
//
//  Created by Harry Kalodner on 3/13/18.
//

#include "equiv_address.hpp"
#include "dedup_address.hpp"

#include <blocksci/util/data_access.hpp>
#include <blocksci/util/hash.hpp>
#include <blocksci/index/address_index.hpp>
#include <blocksci/chain/inout_pointer.hpp>
#include <blocksci/chain/input.hpp>
#include <blocksci/chain/output.hpp>
#include <blocksci/chain/transaction.hpp>
#include <blocksci/scripts/script_variant.hpp>

using namespace blocksci;

namespace std
{
    size_t hash<blocksci::EquivAddress>::operator()(const blocksci::EquivAddress &equiv) const {
        std::size_t seed = 123954;
        for (const auto &address : equiv.addresses) {
            seed ^= address.scriptNum + address.type;
        }
        return seed;
    }
}

EquivAddress::EquivAddress(uint32_t scriptNum, EquivAddressType::Enum type, bool scriptEquivalent_, const DataAccess &access_) : scriptEquivalent(scriptEquivalent_), access(access_) {
    for (auto type : equivAddressTypes(type)) {
        Address address(scriptNum, type, access);
        if (scriptEquivalent) {
            auto nested = access.addressIndex->getPossibleNestedEquivalent(address);
            for (auto &nestedAddress : nested) {
                if (access.addressIndex->checkIfExists(nestedAddress)) {
                    addresses.insert(nestedAddress);
                }
            }
        } else {
            if (access.addressIndex->checkIfExists(address)) {
                addresses.insert(address);
            }
        }
        
    }
}

EquivAddress::EquivAddress(const Address &searchAddress, bool scriptEquivalent_) :
EquivAddress(searchAddress.scriptNum, equivType(searchAddress.type), scriptEquivalent_, searchAddress.getAccess()) {}

EquivAddress::EquivAddress(const DedupAddress &searchAddress, bool scriptEquivalent_, const DataAccess &access_) :
EquivAddress(searchAddress.scriptNum, equivType(searchAddress.type), scriptEquivalent_, access_) {}

std::string EquivAddress::toString() const {
    std::stringstream ss;
    ss << "EquivAddress(";
    size_t i = 0;
    for (auto &address : addresses) {
        ss << address.getScript().toString();
        if (i < addresses.size() - 1) {
            ss << ", ";
        }
        i++;
    }
    ss << ")";
    return ss.str();
}

std::vector<OutputPointer> EquivAddress::getOutputPointers() const {
    std::vector<OutputPointer> outputs;
    for (const auto &address : addresses) {
        auto addrOuts = access.addressIndex->getOutputPointers(address);
        outputs.insert(outputs.end(), addrOuts.begin(), addrOuts.end());
    }
    return outputs;
}

uint64_t EquivAddress::calculateBalance(BlockHeight height) const {
    return blocksci::calculateBalance(getOutputPointers(), height, access);
}

std::vector<Output> EquivAddress::getOutputs() const {
    return blocksci::getOutputs(getOutputPointers(), access);
}

std::vector<Input> EquivAddress::getInputs() const {
    return blocksci::getInputs(getOutputPointers(), access);
}

std::vector<Transaction> EquivAddress::getTransactions() const {
    return blocksci::getTransactions(getOutputPointers(), access);
}

std::vector<Transaction> EquivAddress::getOutputTransactions() const {
    return blocksci::getOutputTransactions(getOutputPointers(), access);
}

std::vector<Transaction> EquivAddress::getInputTransactions() const {
    return blocksci::getInputTransactions(getOutputPointers(), access);
}


