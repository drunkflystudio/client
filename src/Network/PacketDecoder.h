#ifndef NETWORK_PACKETDECODER_H
#define NETWORK_PACKETDECODER_H

#include <QProtobufSerializer>
#include <new>

struct IPacketDecoder
{
    virtual ~IPacketDecoder() = default;
    virtual size_t packetSize() const = 0;
    virtual bool decode(QProtobufSerializer& serializer, void* dst, const QByteArray& data) const = 0;
    virtual void freePacket(void* dst) const = 0;
};

template <typename T> class PacketDecoder final : public IPacketDecoder
{
public:
    PacketDecoder() = default;
    ~PacketDecoder() override = default;

    static const PacketDecoder* instance()
    {
        return &m_instance;
    }

    size_t packetSize() const override
    {
        return sizeof(T);
    }

    bool decode(QProtobufSerializer& serializer, void* dst, const QByteArray& data) const override
    {
        T* packet = new (dst) T();
        try {
            packet->deserialize(&serializer, data);
            if (serializer.deserializationError() == QAbstractProtobufSerializer::NoError)
                return true;
        } catch (...) {
            packet->~T();
            throw;
        }

        packet->~T();
        return false;
    }

    void freePacket(void* dst) const override
    {
        reinterpret_cast<T*>(dst)->~T();
    }

private:
    static const PacketDecoder m_instance;

    Q_DISABLE_COPY_MOVE(PacketDecoder)
};

template <typename T>
const PacketDecoder<T> PacketDecoder<T>::m_instance;

#endif
