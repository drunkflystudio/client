#ifndef NETWORK_CONNECTION_H
#define NETWORK_CONNECTION_H

#include "Network/PacketDecoder.h"
#include <QTimer>
#include <QHash>
#include <QProtobufSerializer>
#include <QStack>
#include <QPointer>
#include <QVector>
#include <functional>

class QWebSocket;

namespace Protobuf { class Packet; };
template <typename T> constexpr int ProtobufMsgID();

class Connection final : public QObject
{
    Q_OBJECT

    struct Listener
    {
        QPointer<QObject> receiver;
        std::function<void(const void*)> callback;
    };

    struct ListenerList
    {
        const IPacketDecoder* decoder;
        QVector<Listener> list;
    };

    struct Callback
    {
        int msgID;
        const IPacketDecoder* decoder;
        QPointer<QObject> receiver;
        std::function<void(const void*)> callback;
        QTimer* timer;
    };

public:
    explicit Connection(QWebSocket* socket);
    ~Connection() override;

    template <typename T> void send(const T& data)
        { sendPacket(ProtobufMsgID<T>(), data.serialize(&m_serializer)); }
    template <typename T> void send(const T& data, QObject* r, std::function<void(const typename T::Response*)> cb)
        { sendPacketWithCB(ProtobufMsgID<T>(), data.serialize(&m_serializer), r,
            PacketDecoder<typename T::Response>::instance(),
            [this, cb](const void* data) { cb(reinterpret_cast<const typename T::Response*>(data)); }); }

    template <typename T> void addListener(QObject* receiver, std::function<void(const T&)> cb)
        { registerListener(ProtobufMsgID<T>(), receiver, PacketDecoder<T>::instance(),
            [this, cb](const void* data){ cb(*reinterpret_cast<const T*>(data)); }); }
    template <typename T> void removeListener(QObject* receiver)
        { unregisterListener(ProtobufMsgID<T>(), receiver); }

signals:
    void onProtocolError();
    void onSerializationError();
    void onServerError();

private:
    QWebSocket* m_socket;
    QHash<int, ListenerList> m_listeners;
    QHash<qint64, Callback> m_pendingCallbacks;
    QStack<QTimer*> m_freeTimers;
    QProtobufSerializer m_serializer;
    qint64 m_nextSeq;
    bool m_iterating;

    void sendPacket(int msgID, const QByteArray& data);
    void sendPacketWithCB(int msgID, const QByteArray& data,
        QObject* r, const IPacketDecoder* decoder, std::function<void(const void*)> cb);

    void registerListener(int msgID, QObject* r, const IPacketDecoder* decoder, std::function<void(const void*)> cb);
    bool unregisterListener(int msgID, QObject* r);

    void onMessageReceived(const QByteArray& data);
    void onResponseReceived(qint64 seq, const Protobuf::Packet* packet);

    Q_DISABLE_COPY_MOVE(Connection)
};

#endif
