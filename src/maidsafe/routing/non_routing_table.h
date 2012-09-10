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

#ifndef MAIDSAFE_ROUTING_NON_ROUTING_TABLE_H_
#define MAIDSAFE_ROUTING_NON_ROUTING_TABLE_H_

#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

#include "boost/asio/ip/udp.hpp"

#include "maidsafe/common/rsa.h"

#include "maidsafe/routing/api_config.h"
#include "maidsafe/routing/node_id.h"


namespace maidsafe {

namespace routing {

struct NodeInfo;

namespace test { class GenericNode; }

namespace protobuf { class Contact; }

class NonRoutingTable {
 public:
  explicit NonRoutingTable(const asymm::Keys& keys);
  bool AddNode(NodeInfo& node, const NodeId& furthest_close_node_id);
  bool CheckNode(NodeInfo& node, const NodeId& furthest_close_node_id);
  NodeInfo DropNode(const NodeId &node_to_drop);
  std::vector<NodeInfo> GetNodesInfo(const NodeId& node_id) const;
  bool IsConnected(const NodeId& node_id) const;

  friend class test::GenericNode;

 private:
  NonRoutingTable(const NonRoutingTable&);
  NonRoutingTable& operator=(const NonRoutingTable&);
  bool AddOrCheckNode(NodeInfo& node, const NodeId& furthest_close_node_id, const bool& add);
  bool CheckValidParameters(const NodeInfo& node) const;
  bool CheckParametersAreUnique(const NodeInfo& node) const;
  bool CheckRangeForNodeToBeAdded(NodeInfo& node,
                                  const NodeId& furthest_close_node_id,
                                  const bool& add) const;
  bool IsThisNodeInRange(const NodeId& node_id, const NodeId& furthest_close_node_id) const;

  const NodeId kNodeId_;
  std::vector<NodeInfo> nodes_;
  mutable std::mutex mutex_;
};

}  // namespace routing

}  // namespace maidsafe

#endif  // MAIDSAFE_ROUTING_NON_ROUTING_TABLE_H_
