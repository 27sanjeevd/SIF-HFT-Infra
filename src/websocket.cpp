#include "../include/websocket.hpp"

WebsocketConnection::WebsocketConnection(std::shared_ptr<Orderbook> new_book, std::string &ws_id) 
    : curr_book_(new_book), id_(ws_id) {

    curr_book_->initialize_exchange(id_);
}

void WebsocketConnection::Initialize() {
    if (!SSL_set_tlsext_host_name(ws_.next_layer().native_handle(), GetHost().c_str())) {
        throw boost::system::system_error(
            static_cast<int>(::ERR_get_error()), boost::asio::error::get_ssl_category());
    }
}

void WebsocketConnection::Connect(std::string &currency_name, std::string &channel_name) {
    try {
        EstablishConnection();
        SubscribeToChannel(currency_name, channel_name);
        StartMessageLoop();
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

void WebsocketConnection::EstablishConnection() {
    tcp::resolver resolver{ioc_};
    auto const results = resolver.resolve(GetHost(), "443");
    net::connect(ws_.next_layer().next_layer(), results);
    
    ws_.next_layer().handshake(ssl::stream_base::client);
    ws_.handshake(GetHost(), "/ws");
}

void WebsocketConnection::StartMessageLoop() {
    beast::flat_buffer buffer;
    
    while (true) {
        ws_.read(buffer);
        ProcessMessage(buffer);
        buffer.consume(buffer.size());
    }
}

void WebsocketConnection::ProcessMessage(const beast::flat_buffer& buffer) {
    std::string message(
        static_cast<const char*>(buffer.data().data()),
        buffer.data().size()
    );
    HandleMessage(message);
}

void WebsocketConnection::Disconnect() {
    try {
        ws_.close(websocket::close_code::normal);
    } catch (std::exception const& e) {
        std::cerr << "Error closing connection: " << e.what() << std::endl;
    }
}