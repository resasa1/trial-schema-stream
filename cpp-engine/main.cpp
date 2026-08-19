#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <iostream>
#include <thread>
#include <chrono>
#include <fstream>
#include <vector>
#include <mutex>

typedef websocketpp::server<websocketpp::config::asio> server;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::bind;


class VideoSteamer {
	private:
		server ws_server;
		std::vector<websocketpp::connection_hdl> clients;
		std::mutex clients_mutex;
		std::atomic<bool> streaming{true};

		std::vector<uint8_t> video_buffer;

	public:
		VideoStreamer(){
			ws_server.init_asio();
			ws_server.set_open_handler(bind(&VideoStreamer::on_open, this, ::_1));
			ws_server.set_close_handler(bind(&VideoStreamer::on_close, this, ::_1));
			ws_server.set_message_handler(bind(&VideoStreamer::on_message, this, ::_1, ::_2));
			
			video_buffer.reserve(1024 * 1024); // Reserve 1MB for video buffer
			for(int i = 0; i < video_buffer.size(); i++) {
				video_buffer[i] = rand() % 256;
			}
		}

		void on_open(websocketpp::connection_hdl hdl) {
			std::lock_guard<std::mutex> lock(clients_mutex);
			clients.push_back(hdl);
			std::cout << "Client connected. Total: " << clients.size() << std::endl;
		}

		void on_close(websocketpp::connection_hdl hdl)
			std::lock_guard<std::mutex> lock(clients_mutex);
			clients.erase(std::remove_if(clients.begin(), clients.end(), 
				[hdl](websocketpp::connection_hdl h) { 
					return h.owner_before(hdl) && !hdl.owner_before(h);
				}), clients.end());
			std::cout << "Client disconnected. Total: " << clients.size() << std::endl;
		}

		void on_message(websocketpp::connection_hdl hdl, server::message_ptr msg) {
			std::string cmd = msg->get_payload();
			std::cout << "Received message: " << cmd << std::endl;
		}

		void broadcast_frame() {
			while(streaming) {
				std::lock_guard<std::mutex> lock(clients_mutex);

				//simulasi encoding & buffering
				std::vector<uint8_t> frame = video_buffer; // copy buffer

				for(auto hdl : clients) {
					try {
						ws_server.send(hdl, frame.data(), frame.size(), websocketpp::frame::opcode::binary);
					} catch (...) {
						//disconnect clinet will be handle by on_close
					}
				}
				std::this_thread::sleep_for(std::chrono::milliseconds(33));
			}
		}

		void run(int port = 8081) {
        std::cout << "🚀 C++ Stream Engine starting on port " << port << std::endl;
        ws_server.listen(port);
        ws_server.start_accept();
        
        // Jalankan broadcast di thread terpisah
        std::thread broadcast_thread(&VideoStreamer::broadcast_frame, this);
        
        ws_server.run();
        streaming = false;
        broadcast_thread.join();
    }

int main() {
    VideoStreamer streamer;
    streamer.run(8081);
    return 0;
}