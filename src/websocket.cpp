#include "../include/websocket.hpp"

WebsocketConnection::WebsocketConnection(std::shared_ptr<Orderbook> new_book, std::string &ws_id,
    std::shared_ptr<std::mutex> mutex) : curr_book_(new_book), id_(ws_id), mutex_(mutex) {

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
    ws_.handshake(GetHost(), GetTarget());
}

void WebsocketConnection::StartMessageLoop() {

    while (true) {
        //auto now = std::chrono::system_clock::now();

        ws_.read(buffer_);
        ProcessMessage(buffer_);
        buffer_.consume(buffer_.size());

        //CalculateRoundTime(now);
    }
}

void WebsocketConnection::ProcessMessage(const beast::flat_buffer& buffer) {
    std::string_view message(
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

void WebsocketConnection::CalculateRoundTime(std::chrono::system_clock::time_point start_time) {
    auto now = std::chrono::system_clock::now();
    auto time_diff = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count();

    if (time_diff < 1000) {
        if (time_list.size() < 50) {
            average_time += static_cast<double>(time_diff);
            time_list.push_back(time_diff);
        } 
        else {
            average_time += static_cast<double>(time_diff) - time_list.front();
            time_list.pop_front();
            time_list.push_back(time_diff);
        }
    }

    std::cout << (average_time) / time_list.size() << "\n";
}

bool WebsocketConnection::ConvertToDouble(std::string_view sv, double &result) {
    std::string temp(sv);
    char* endptr = nullptr;
    result = std::strtod(temp.c_str(), &endptr);
    return (endptr && static_cast<size_t>(endptr - temp.c_str()) == temp.size());
}