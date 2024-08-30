
class ServerOfflineState final : public Server::State
{
    Q_OBJECT

public:
    explicit ServerOfflineState(Server* server)
        : State(server, Server::Offline)
    {
    }

    QString statusText() const override
    {
        return tr("Offline");
    }

    Q_DISABLE_COPY_MOVE(ServerOfflineState)
};
