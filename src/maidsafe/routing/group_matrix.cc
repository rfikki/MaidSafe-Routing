/*******************************************************************************
 *  Copyright 2012 maidsafe.net limited                                        *
 *                                                                             *
 *  The following source code is property of maidsafe.net limited and is not   *
 *  meant for external use.  The use of this code is governed by the licence   *
 *  file licence.txt found in the root of this directory and also on           *
 *  www.maidsafe.net.                                                          *
 *                                                                             *
 *  You are not free to copy, amend or otherwise use this source code without  *
 *  the explicit written permission of the board of directors of maidsafe.net. *
 ******************************************************************************/

#include "maidsafe/routing/group_matrix.h"

#include <algorithm>

#include "maidsafe/common/log.h"

#include "maidsafe/routing/parameters.h"
#include "maidsafe/routing/node_info.h"
#include "maidsafe/routing/return_codes.h"

namespace maidsafe {

namespace routing {

GroupMatrix::GroupMatrix(const NodeId& this_node_id)
    : kNodeId_(this_node_id),
      mutex_(),
      unique_nodes_(),
      matrix_() {}

void GroupMatrix::AddConnectedPeer(const NodeInfo& node_info) {
  std::lock_guard<std::mutex> lock(mutex_);
  LOG(kInfo) << "AddConnectedPeer : " << DebugId(node_info.node_id);
  for (auto nodes : matrix_) {
    if (nodes.at(0).node_id == node_info.node_id) {
      LOG(kWarning) << "Already Added in matrix";
      return;
    }
  }
  if (matrix_.size() >= Parameters::closest_nodes_size) {
    assert(false && "Matrix should not have more than Parameters::closest_nodes_size rows.");
    return;
  }
  matrix_.push_back(std::vector<NodeInfo>(1, node_info));
//  LOG(kVerbose) << "matrix_.size(): " << matrix_.size();
  UpdateUniqueNodeList();
}

void GroupMatrix::RemoveConnectedPeer(const NodeInfo& node_info) {
  std::lock_guard<std::mutex> lock(mutex_);
  matrix_.erase(std::remove_if(matrix_.begin(),
                               matrix_.end(),
                               [node_info](const std::vector<NodeInfo> nodes) {
                                   return (node_info.node_id == nodes.at(0).node_id);
                               }), matrix_.end());
  UpdateUniqueNodeList();
}

std::vector<NodeInfo> GroupMatrix::GetConnectedPeers() {
  std::lock_guard<std::mutex> lock(mutex_);
  std::vector<NodeInfo> connected_peers;
  for (auto nodes : matrix_)
    connected_peers.push_back(nodes.at(0));
  PartialSortFromTarget(kNodeId_, Parameters::closest_nodes_size, connected_peers);
  return connected_peers;
}

NodeInfo GroupMatrix::GetConnectedPeerFor(const NodeId& target_node_id) {
  std::lock_guard<std::mutex> lock(mutex_);

  for (auto nodes : matrix_) {
    if (nodes.at(0).node_id == target_node_id) {
      assert(false && "Shouldn't request connected peer for node in first column of group matrix.");
      return NodeInfo();
    }
  }
  for (auto nodes : matrix_) {
    if (std::find_if(nodes.begin(), nodes.end(),
                     [target_node_id](const NodeInfo& node_info) {
                       return (node_info.node_id == target_node_id);
                     }) != nodes.end()) {
      return nodes.at(0);
    }
  }
  return NodeInfo();
}

NodeId GetConnectedPeerClosestTo(const NodeId& target_node_id) {
  // TODO:(Prakash) : Implement
  return target_node_id;
}

bool GroupMatrix::IsThisNodeGroupMemberFor(const NodeId& target_id, bool& is_group_leader) {
  std::lock_guard<std::mutex> lock(mutex_);
  is_group_leader = false;
  if (unique_nodes_.empty()) {
    is_group_leader = true;
    return true;
  }

  PartialSortFromTarget(target_id, Parameters::node_group_size, unique_nodes_);

  if (unique_nodes_.size() < Parameters::node_group_size) {
    if (unique_nodes_.at(0).node_id == kNodeId_) {
      is_group_leader = true;
    }
    return true;
  }

  NodeInfo furthest_group_node(unique_nodes_.at(Parameters::node_group_size - 1));
  if ((furthest_group_node.node_id ^ target_id) >=
      (kNodeId_ ^ target_id)) {
    if (unique_nodes_.at(0).node_id == kNodeId_)
      is_group_leader = true;
    return true;
  }
  return false;
}

void GroupMatrix::UpdateFromConnectedPeer(const NodeId& peer,
                                          const std::vector<NodeInfo>& nodes) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (peer.IsZero()) {
    assert(false && "Invalid peer node id.");
    return;
  }
  if (nodes.size() >= Parameters::closest_nodes_size) {
    assert(false && "Vector of nodes should have length less than Parameters::closest_nodes_size");
    return;
  }
  // If peer is in my group
  auto group_itr(matrix_.begin());
  for (group_itr = matrix_.begin(); group_itr != matrix_.end(); ++group_itr) {
    if ((*group_itr).at(0).node_id == peer)
      break;
  }

  if (group_itr == matrix_.end()) {
    LOG(kWarning) << "Peer Node : " << DebugId(peer)
                  << " is not in closest group of this node.";
    return;
  }

  // Update peer's row
  // group_itr->clear();
  if (group_itr->size() > 1) {
    group_itr->erase(group_itr->begin() + 1, group_itr->end());
  }
//  group_itr->push_back(peer);
  for (auto i : nodes)
    group_itr->push_back(i);

  // Update unique node vector
  UpdateUniqueNodeList();
}

bool GroupMatrix::GetRow(const NodeId& row_id, std::vector<NodeInfo>& row_entries) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (row_id.IsZero()) {
    assert(false && "Invalid node id.");
    return false;
  }
  auto group_itr(matrix_.begin());
  for (group_itr = matrix_.begin(); group_itr != matrix_.end(); ++group_itr) {
    if ((*group_itr).at(0).node_id == row_id)
      break;
  }

  if (group_itr == matrix_.end())
    return false;

  row_entries.clear();
  for (uint32_t i(0); i < (*group_itr).size(); ++i) {
    if (i != 0) {
      row_entries.push_back((*group_itr).at(i));
    }
  }
  return true;
}

std::vector<NodeInfo> GroupMatrix::GetClosestNodes(const uint32_t& size) {
  std::lock_guard<std::mutex> lock(mutex_);
  return std::vector<NodeInfo>(unique_nodes_.begin(),
                             unique_nodes_.begin() + std::min(size, unique_nodes_.size()));
}

std::vector<NodeInfo> GroupMatrix::GetUniqueNodes() {
  std::lock_guard<std::mutex> lock(mutex_);
  return unique_nodes_;
}

void GroupMatrix::Clear() {
  matrix_.clear();
  UpdateUniqueNodeList();
}

void GroupMatrix::UpdateUniqueNodeList() {
  unique_nodes_.clear();
  // Update unique node vector
  NodeInfo node_info;
  node_info.node_id = kNodeId_;
  unique_nodes_.push_back(node_info);
  for (auto itr = matrix_.begin(); itr != matrix_.end(); ++itr)
    for (size_t i(0); i !=  itr->size(); ++i)
      unique_nodes_.push_back((*itr).at(i));

  // Removing duplicates
  std::sort(unique_nodes_.begin(),
            unique_nodes_.end(),
            [=](const NodeInfo& lhs, const NodeInfo& rhs) {
              return (lhs.node_id ^ kNodeId_) < (rhs.node_id ^ kNodeId_);
            });

  auto itr = std::unique(unique_nodes_.begin(),
                         unique_nodes_.end(),
                         [](const NodeInfo& lhs, const NodeInfo& rhs) {
                           return lhs.node_id == rhs.node_id;
                         });
  unique_nodes_.resize(itr - unique_nodes_.begin());
}

void GroupMatrix::PartialSortFromTarget(const NodeId& target,
                                        const uint16_t& number,
                                        std::vector<NodeInfo>& nodes) {
  uint16_t count = std::min(number, static_cast<uint16_t>(nodes.size()));
  std::partial_sort(nodes.begin(),
                    nodes.begin() + count,
                    nodes.end(),
                    [target](const NodeInfo& lhs, const NodeInfo& rhs) {
                      return NodeId::CloserToTarget(lhs.node_id, rhs.node_id, target);
                    });
}

void GroupMatrix::PrintGroupMatrix() {
  std::lock_guard<std::mutex> lock(mutex_);
  auto group_itr(matrix_.begin());
  std::string tab("\t");
  std::string output("Group matrix of node with NodeID: " + DebugId(kNodeId_));
  for (group_itr = matrix_.begin(); group_itr != matrix_.end(); ++group_itr) {
    output.append("\nGroup matrix row:");
    for (uint32_t i(0); i < (*group_itr).size(); ++i) {
      output.append(tab);
      output.append(DebugId((*group_itr).at(i).node_id));
    }
  }
  LOG(kVerbose) << output;
}



}  // namespace routing

}  // namespace maidsafe
