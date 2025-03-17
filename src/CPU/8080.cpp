#include "8080.hpp"

_8080::_8080() {
  screen = new Screen();
  memory = (u8*) malloc(sizeof(u8) * TOTAL_BYTES_OF_MEM);

}

_8080::~_8080() {

}

void _8080::load_rom(const string& file_path, u16 start_address) {
  
  // Open file in binary mode
  ifstream romFile(file_path, ios::binary);

  if (!romFile) {
    cerr << "Error: could not open ROM file " << file_path << endl;
    return;  
  }

  // Get the size of the file by shifting pointer to end
  romFile.seekg(0, ios::end);
  streampos size = romFile.tellg();
  romFile.seekg(0, ios::beg);

  // Allocate memory for the buffer using a vector for automatic memory management
  vector<char> buffer(size);

  // Read the contents of the file into the buffer
  romFile.read(buffer.data(), size);
  
  if (!romFile) {
    std::cerr << "Error: failed to read the entire ROM file" << std::endl;
    return;
  }

  romFile.close();

  // Copy the ROM data into memory starting at specified starting adress
  for (std::size_t i = 0; i < size; ++i) {
    memory[start_address + i] = buffer[i];
  }
}

void _8080::run() {

  SDL_Event event;
  bool running = true;

  while (running) {

    while( SDL_PollEvent( &event ) ){
      switch( event.type ){
        case SDL_QUIT:  
          running = false;
          break;
        case SDL_KEYDOWN:
          handle_key_press(event.key.keysym.sym);
          break;
        case SDL_KEYUP:
          handle_key_release(event.key.keysym.sym);
          break;
        default:
          break;
      }
    }
    
    screen->render_screen();

    SDL_Delay(1);
  }
}