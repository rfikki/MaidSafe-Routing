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

#ifndef MAIDSAFE_ROUTING_SERVICE_H_
#define MAIDSAFE_ROUTING_SERVICE_H_

#include <memory>

#include "maidsafe/routing/api_config.h"

namespace maidsafe {

namespace routing {

namespace protobuf { class Message;}  // namespace protobuf

class NonRoutingTable;
class RoutingTable;

namespace service {

// Handle all incoming requests and send back reply
void Ping(RoutingTable &routing_table,
          protobuf::Message &message);
void Connect(RoutingTable &routing_table,
             NonRoutingTable &non_routing_table,
             rudp::ManagedConnections &rudp,
             protobuf::Message &message,
             RequestPublicKeyFunctor node_validation_functor);
void FindNodes(RoutingTable &routing_table,
              protobuf::Message &message);
void ProxyConnect(RoutingTable &routing_table,
                  rudp::ManagedConnections &rudp,
                  protobuf::Message &message);

}  // namespace service

}  // namespace routing

}  // namespace maidsafe


#endif  // MAIDSAFE_ROUTING_SERVICE_H_