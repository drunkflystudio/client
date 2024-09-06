/* THIS IS AN AUTOMATICALLY GENERATED FILE. DO NOT CHANGE. */
#ifndef NETWORK_PROTOCOL_H
#define NETWORK_PROTOCOL_H

#include "Network/Connection.h"
#include "Protobuf/Project.qpb.h"
#include "Protobuf/_Common.qpb.h"
#include "Protobuf/_MsgIDs.qpb.h"

template <> constexpr int ProtobufMsgID<Protobuf::CreateProject>()
    { return Protobuf::MsgIDGadget::MsgID_CreateProject; }

template <> constexpr int ProtobufMsgID<Protobuf::CreateProject::Response>()
    { return Protobuf::MsgIDGadget::MsgID_CreateProject_Response; }

template <> constexpr int ProtobufMsgID<Protobuf::Project>()
    { return Protobuf::MsgIDGadget::MsgID_Project; }

#endif
