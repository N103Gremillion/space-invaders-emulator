#include "8080.hpp"
#include <unistd.h>

#define BLACK_FONT 0, 0, 0, 255
#define WHITE_FONT 255, 255, 255, 255


string get_hex_string(int num) {
  // width is 6 since 
  std::stringstream stream;
  stream << std::setfill('0') << setw(6) << std::hex << num;
  return stream.str();
}

_8080::_8080() {
  TTF_Init();
  SDL_Init(SDL_INIT_VIDEO);

  memory = (u8*) malloc(sizeof(u8) * TOTAL_BYTES_OF_MEM);
  regs = new Registers();
  screen = new Screen();
  window = SDL_CreateWindow("Instructions", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,  window_w, window_h, SDL_WINDOW_SHOWN);
  
  // shift to correct position
  int window_x;
  int window_y;
  SDL_GetWindowPosition(window, &window_x, &window_y);
  SDL_SetWindowPosition(window, window_x * 0.3, window_y * 0.3);
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

}

_8080::~_8080() {
  SDL_DestroyWindow(window);
  delete[] memory;
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

void _8080::fill_background() {
  SDL_SetRenderDrawColor(renderer, BLACK_FONT);
  SDL_RenderClear(renderer);
  SDL_SetRenderDrawColor(renderer, WHITE_FONT);
}

void _8080::draw_instructions() {
  int x = 0;
  int y = 0;
  SDL_Color color = {255, 255, 255};
  u16 temp = regs->pc;
  int instructions_to_draw = 35;

  for (int i = 0; i < instructions_to_draw; i++){
    u32 instruction = memory[temp + i];
    int index = temp + i;
    if (instruction_list[int(instruction)] == 2){
      temp += 1;
      instruction = (instruction << 8) | memory[temp + i];
    } else if (instruction_list[int(instruction)] == 3) {
      temp += 1;
      instruction = (instruction << 8) | memory[temp + i];
      temp += 1;
      instruction = (instruction << 8) | memory[temp + i];
    }
    string instruction_text = get_hex_string(index) + ": 0x" + get_hex_string(instruction);
    SDL_Surface* text = TTF_RenderText_Solid(regs->font, instruction_text.c_str(), color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface( renderer, text );
    SDL_Rect text_rect = {x, y, text->w, text->h};
    SDL_RenderCopy(renderer, texture, NULL, &text_rect);
    y += 20;
    SDL_DestroyTexture( texture );
    SDL_FreeSurface( text ); 
  } 
  SDL_RenderPresent(renderer);
}

void _8080::render() {
  // render all screens
  screen->render_screen();
  fill_background();
  draw_instructions();
  regs->render_regs();
}

void _8080::run() {

  SDL_Event event;
  int open_windows = 3;
  bool running = true;

  while (running) {

    // event handling
    while( SDL_PollEvent( &event ) ){
      switch( event.type ){
        case SDL_QUIT:  
          running = false;
          break;
        case SDL_WINDOWEVENT: {
          if(event.window.event != SDL_WINDOWEVENT_CLOSE) {
            break;
          }
          SDL_Window* closed_window = SDL_GetWindowFromID(event.window.windowID);
          if (closed_window == window) {
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
          }
          else if (closed_window == screen->window) {
            SDL_DestroyRenderer(screen->renderer);
            SDL_DestroyWindow(screen->window);
          }
          else if (closed_window == regs->window) {
            SDL_DestroyRenderer(screen->renderer);
            SDL_DestroyWindow(regs->window);
          }
          open_windows--;
        }
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

    // graphics handling
    render();

    // for debugging
    cout << "Press any key to continue...";
    cin.get();

    // instruction handling
    u8 opcode = fetch_byte();
    execute_instruction(opcode);

    SDL_Delay(1000);
  }
}

// use pc to get the next byte in memory
u8 _8080::fetch_byte() {
  u8 opcode = memory[regs->pc];
  regs->pc += 1;
  return opcode;
}

// check type of instruciotn using opcode and perform instruciton
void _8080::execute_instruction(u8 opcode) {
  switch (opcode) {

    // 00 - 0F
    // NOP / nothing instruciton
    case 0x00:
      printf("NOP do nothing \n");
      break;
    // LXI B, d16 / load preciding 16 bits into register BC
    case 0x01:
      printf("Load next 16 bits into BC \n");
      break;
    // STAX (store accumulator inderectly) B / store value of A reg into memory location pointed to by BC reg_pair
    case 0x02:
      printf("store value in A reg into mem location pointed to by BC reg_pair \n");
      break;
    // INX B (increment reg pair) / increment BC reg pair by 1
    case 0x03:
      printf("Increment BC reg_pair by 1 \n");
      break;
    // INR B (incrment reg) / increment B reg by 1
    case 0x04:
      printf("increment B reg by 1 \n");
      break;
    // DCR B (decrement reg) / decrement B reg by 1
    case 0x05:
      printf("decrement B reg by 1 \n");
      break;
    // MVI B, d8 (move immediate) / move d8 value into B reg
    case 0x06:
      printf("move the proceding 8 bits into B reg \n");
      break;
    // RLC (Rotate left through carry) / shift bits of A by 1 (A << 1) then set LSB (least sig bit) of A to value in carry finally take the MSB (most sig bit) of A and make carry that value
    case 0x07:
      printf("shift bits of A by 1 (A << 1) then set LSB (least sig bit) of A to value in carry finally take the MSB (most sig bit) of A and make carry that value \n");
      break;
    // another NOP
    case 0x08:
      printf("another NOP \n");
      break;
    // DAD B (double add) / add value in BC reg pair to HL reg pair (modifies the carry flag if there is overflow)
    case 0x09:
      printf("add value in BC reg pair to HL reg pair (modifies the carry flag if there is overflow) \n");
      break;
    // LDAX B (load accumulator from mem) / load memory address pointed to by BC (memory[BC]) into A reg 
    case 0x0A:
      printf("load memory address pointed to by BC (memory[BC]) into A reg \n");
      break;
    // DCX B (decremnt hex value(16 bit)) / decremtn BC reg pair by 1
    case 0x0B:
      printf("decrement BC reg pair by 1 \n");
      break;
    // INC C / incremtent c by 1 (note affects many flags)
    case 0x0C:
      printf("incremtent c by 1 (note affects many flags) \n");
      break;
    // DCR C / decrement c by 1
    case 0x0D:
      printf("decrement c by 1 \n");
      break;
    // MVI, C, d8 / move next byte into C reg
    case 0x0E:
      printf("move next byte into C reg \n");
      break;
    // RRC / rotate right through carry 
    case 0x0F:
      printf("rotate right through carry \n");
      break;


    // 10 - 1F
    // NOP command
    case 0x10:
      printf("nothing \n");
      break;


    // 20 - 2F
    // NOP command
    case 0x20:
      printf("nothing \n");
      break;


    // 30 - 3F
    case 0x30:
      printf("nothing \n");
      break;


    // 40 - 4F
    // MOV B,B / moves B reg into B
    case 0x40:
      printf("moves B reg into B \n");
      break;


    // 50 - 5F
    // MOV D,B / moves B into D
    case 0x50:
      printf("moves B reg into D reg \n");
      break;


    // 60 - 6F
    // MOV H,B / moves B into H
    case 0x60:
      printf("moves B reg into H reg \n");
      break;


    // 70 - 7F
    // MOV M,B / moves contents in B into memory location in HL
    case 0x70:
      printf("moves contents in B into memory location in HL \n");
      break;


    // 80 - 8F
    // ADD B / adds contents of B into A
    case 0x80:
      printf("A = A + B \n");
      break;


    // 90 - 9F
    // SUB B / subtracts the contents of B from A
    case 0x90:
      printf("A = A - B \n");
      break;
     

    // A0 - AF
    // ANA B / bitwize and & between A and B stored in A
    case 0xA0:
      printf("A = A & B. \n");
      break;


    // B0 - BF
    // ORA B / bitwize or | between A and B and stored in A
    case 0xB0:
      printf("A = A | B. \n");
      break;


    // C0 - CF
    // RNZ (return if zero) / checks the zero flag is 0 pop 2 bytes from stack(address) and set the PC to this location 
    case 0xC0:
      printf("checks the zero flag is 0 pop 2 bytes from stack(address) and set the PC to this location. \n");
      break;
    
    // JMP a16  / 3 bytes / 10 cycles / - - - - - / uncondition jump to the mem address given by next 2 bytes in memory  
    case 0xC3: {
      u16 mem_location = (fetch_byte() << 8) | fetch_byte();
      regs->pc = mem_location;
      cycles += 10;
    }





    // D0 - DF
    // RNC / If the carry bit is zero, a return (pop 2 bytes from stack to get) operation is performed.
    case 0xD0:
      printf("If the carry bit is zero, a return operation is performed. \n");
      break;


    // E0 - EF
    // RPO / If the Parity bit is zero (indicating odd parity), a return (pop 2 bytes form stack and set pc to it) operation is performed.
    case 0xE0:
      printf("If the Parity bit is zero (indicating odd parity), a return operation is performed. \n");
      break;


    // F0 - FF
    // RP / pc = (2 bytes poped from stack) if (sign flag is 0)
    case 0xF0:
      printf("If the Sign bit is zero (indicating a positive result). a return operation is performed. \n");  
      break;
    // POP PSW / PSW = (pop 2 bytes from stack)
    case 0xF1:
      printf("set PSW = to the top 2 bytes on the stack by poping them. \n");
      break;
    // JP a16 (jump if positive)/ pc = next 2 bytes in memory if sign flag = 0
    case 0xF2:
      printf("pc = next 2 bytes in memory if sign flag = 0. \n");
      break;
    // DI (dissable interupts)
    case 0xF3:
      printf("disable interrupts, preventing the processor from responding to interrupt requests. \n");
      break;
    // CP a16 / 
    case 0xF4:
      printf(" ");
      break;
    //
    case 0xF5:
      printf(" ");
      break;
    //
    case 0xF6:
      printf(" ");
      break;
    //
    case 0xF7:
      printf(" ");
      break;
    //
    case 0xF8:
      printf(" ");
      break;
    //
    case 0xF9:
      printf(" ");
      break;
    //
    case 0xFA:
      printf(" ");
      break;
    //
    case 0xFB:
      printf(" ");
      break;
    //
    case 0xFC:
      printf(" ");
      break;
    //
    case 0xFD:
      printf(" ");
      break;
    //
    case 0xFE:
      printf(" ");
      break;
    //
    case 0xFF:
      printf(" ");
      break;
  }
}

