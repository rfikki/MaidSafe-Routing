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

#ifndef MAIDSAFE_ROUTING_RESPONSE_HANDLER_H_
#define MAIDSAFE_ROUTING_RESPONSE_HANDLER_H_

#include "boost/thread/shared_mutex.hpp"
#include "boost/thread/mutex.hpp"

#include "maidsafe/common/rsa.h"
#include "maidsafe/rudp/managed_connections.h"

#include "maidsafe/routing/log.h"
#include "maidsafe/routing/node_id.h"
#include "maidsafe/routing/rpcs.h"

namespace maidsafe {

namespace routing {

namespace protobuf { class Message; }  // namespace protobuf

class NonRoutingTable;
class RoutingTable;

namespace response {

void Ping(protobuf::Message &message);
void Connect(RoutingTable &routing_table,
             NonRoutingTable &non_routing_table,
             rudp::ManagedConnections &rudp,
             protobuf::Message &message,
             RequestPublicKeyFunctor node_validation_functor);
void FindNode(RoutingTable &routing_table,
              NonRoutingTable &non_routing_table,
              rudp::ManagedConnections &rudp,
              const protobuf::Message &message,
              const Endpoint &bootstrap_endpoint);
void ProxyConnect(protobuf::Message& message);

}  // namespace response

}  // namespace routing

}  // namespace maidsafe

#endif  // MAIDSAFE_ROUTING_RESPONSE_HANDLER_H_