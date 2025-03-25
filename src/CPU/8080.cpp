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

     // SDL_Delay(1000);
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

    // NOP / 1 byte / 4 cycles / - - - - - /  nothing instruciton
    case 0x00:
      break;

    // LXI B, d16 / load preciding 16 bits into register BC
    case 0x01:
      printf("Load next 16 bits into BC \n");
      break;
    // STAX (store accumulator inderectly) B / store value of A reg into memory location pointed to by BC reg_pair
    case 0x02:
      printf("store value in A reg into mem location pointed to by BC reg_pair \n");
      break;

    // INX B / 1 byte / 5 cycles / - - - - - / (increment reg pair) / increment BC reg pair by 1
    case 0x03:
      regs->bc++;
      break;

    // INR B / 1 byte / 5 cycles / S Z AC P - /  (incrment reg) / increment B reg by 1
    case 0x04:
      regs->ac = check_auxilary_flag(regs->b, 1, ADD);
      regs->b++;
      regs->s = check_sign_flag(regs->b);
      regs->z = check_zero_flag(regs->b);
      regs->p = check_parity_flag(regs->b);
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


    // 40 - 4F ////////////////////////////////////////////////////
    // MOV B,B / 1 byte / 5 cycles / - - - - - / moves B reg into B
    case 0x40:
      regs->b = regs->b;
      break;
    // MOV B, C / 1 byte / 5 cycles / - - - - - / moves C reg val int B
    case 0x41:
      regs->b = regs->c;
      break;
    // MOV B, D / 1 byte / 5 cycles / - - - - - / moves D reg val int B
    case 0x42:
      regs->b = regs->d;
      break;
    // MOV B, E / 1 byte / 5 cycles / - - - - - / moves E reg val int B
    case 0x43:
      regs->b = regs->e;
      break;
    // MOV B, H / 1 byte / 5 cycles / - - - - - / moves H reg val int B
    case 0x44:
      regs->b = regs->h;
      break;
    // MOV B, L / 1 byte / 5 cycles / - - - - - / moves L reg val int B
    case 0x45:
      regs->b = regs->l;
      break;
    // MOV B, M / 1 byte / 7 cycles / - - - - - / moves value form mem locatioin pointed to by HL into B
    case 0x46:
      regs->b = memory[regs->hl];
      break;
    // MOV B, A / 1 byte / 5 cycles / - - - - - / moves A reg val into B
    case 0x47:
      regs->b = regs->a;
      break;
    // MOV C, B / 1 byte / 5 cycles / - - - - - / moves B reg val into C
    case 0x48:
      regs->c = regs->b;
      break;
    // MOV C, C / 1 byte / 5 cycles / - - - - - / moves C reg val into C
    case 0x49:
      regs->c = regs->c;
      break;
    // MOV C, D / 1 byte / 5 cycles / - - - - - / moves D reg val into C
    case 0x4A:
      regs->c = regs->d;
      break;
    // MOV C, E / 1 byte / 5 cycles / - - - - - / moves E reg val into C
    case 0x4B:
      regs->c = regs->e;
      break;
    // MOV C, H / 1 byte / 5 cycles / - - - - - / moves H reg val into C
    case 0x4C:
      regs->c = regs->h;
      break;
    // MOV C, L / 1 byte / 5 cycles / - - - - - / moves L reg val into C
    case 0x4D:
      regs->c = regs->l;
      break;
    // MOV C, M / 1 byte / 7 cycles / - - - - - / moves value in memory location pointed to by HL reg val into C
    case 0x4E:
      regs->c = memory[regs->hl];
      break;
    // MOV C, A / 1 byte / 5 cycles / - - - - - / moves A reg val into C
    case 0x4F:
      regs->c = regs->a;
      break;


    // 50 - 5F ////////////////////////////////////////////////////
    // MOV D,B / 1 byte / 5 cycles /  moves B into D
    case 0x50:
      regs->d = regs->b;
      break;
    // MOV D, C /  1 byte / 5 cycles / moves C into D
    case 0x51:
      regs->d = regs->c;
      break;
    // MOV D, D /  1 byte / 5 cycles / moves D into D
    case 0x52:
      regs->d = regs->d;
      break;
    // MOV D, E /  1 byte / 5 cycles / moves E into D
    case 0x53:
      regs->d = regs->e;
      break;
    // MOV D, H /  1 byte / 5 cycles / moves H into D
    case 0x54:
      regs->d = regs->h;
      break;
    // MOV D, L /  1 byte / 5 cycles / moves L into D
    case 0x55:
      regs->d = regs->l;
      break;
    // MOV D, M /  1 byte / 7 cycles / moves contents in memory location spcified by HL into D reg
    case 0x56:  
      regs->d = memory[regs->hl];
      break;
    // MOV D, A /  1 byte / 5 cycles / moves A into D
    case 0x57:
      regs->d = regs->a;
      break;
    // MOV E, B / 1 byte / 5 cycles / moves B into E
    case 0x58:
      regs->e = regs->b;
      break;
    // MOV E, C / 1 byte / 5 cycles / moves C into E
    case 0x59:
      regs->e = regs->c;
      break;
    // MOV E, D / 1 byte / 5 cycles / moves D into E
    case 0x5A:
      regs->e = regs->d;
      break;
    // MOV E, E / 1 byte / 5 cycles / moves E into E
    case 0x5B:
      regs->e = regs->e;
      break;
    // MOV E, H / 1 byte / 5 cycles / moves H into E
    case 0x5C:
      regs->e = regs->h;
      break;
    // MOV E, L / 1 byte / 5 cycles / moves L into E
    case 0x5D:
      regs->e = regs->l;
      break;
    // MOV E, M / 1 byte / 7 cycles / moves contents in memory location refered to by HL into E
    case 0x5E:
      regs->e = memory[regs->hl];
      break;
    // MOV E, A / 1 byte / 5 cycles / moves the contents of A into E
    case 0x5F:
      regs->e = regs->a;
      break;

    // 60 - 6F ////////////////////////////////////////////////////
    // MOV H,B / 1 byte / 5 cycles / moves B into H
    case 0x60:
      regs->h = regs->b;
      break;
    // MOV H,C / 1 byte / 5 cycles / moves C into H
    case 0x61:
      regs->h = regs->c;
      break;
    // MOV H,D / 1 byte / 5 cycles / moves D into H
    case 0x62:
      regs->h = regs->d;
      break;
    // MOV H,E / 1 byte / 5 cycles / moves E into H
    case 0x63:
      regs->h = regs->e;
      break;
    // MOV H,H / 1 byte / 5 cycles / moves H into H
    case 0x64:
      regs->h = regs->h;
      break;
    // MOV H,L / 1 byte / 5 cycles / moves L into H
    case 0x65:
      regs->h = regs->l;
      break;
    // MOV H,M / 1 byte / 7 cycles / moves the value in memory reference by the value in reg HL and sets it to H
    case 0x66:
      regs->h = memory[regs->hl];
      break;
    // MOV H,A / 1 byte / 5 cycles / moves A into H
    case 0x67:
      regs->h = regs->a;
      break;
    // MOV L,B / 1 byte / 5 cycles / moves B into L
    case 0x68:
      regs->l = regs->b;
      break;
    // MOV L,C / 1 byte / 5 cycles / moves C into L
    case 0x69:
      regs->l = regs->c;
      break;
    // MOV L,D / 1 byte / 5 cycles / moves D into L
    case 0x6A:
      regs->l = regs->d;
      break;
    // MOV L,E / 1 byte / 5 cycles / moves E into L
    case 0x6B:
      regs->l = regs->e;
      break;
    // MOV L,H / 1 byte / 5 cycles / moves H into L
    case 0x6C:
      regs->l = regs->h;
      break;
    // MOV L,L / 1 byte / 5 cycles / moves L into L
    case 0x6D:
      regs->l = regs->l;
      break;
    // MOV L,M / 1 byte / 7 cylces / moves the value in memory referenced by HL into the L reg
    case 0x6E:
      regs->l = memory[regs->hl];
      break;
    // MOV L,A / 1 byte / 5 cycles / moves A into L
    case 0x6F:
      regs->l = regs->a;
      break;


    // 70 - 7F /////////////////////////////////////////////////////
    // MOV M,B / 1 byte / 7 cycles /  moves contents in B into memory location reference by HL
    case 0x70:
      memory[regs->hl] = regs->b;
      break;
    // MOV M,C / 1 byte / 7 cycles /  moves contents in C into memory location reference by HL
    case 0x71:
      memory[regs->hl] = regs->c;
      break;
    // MOV M,D / 1 byte / 7 cycles /  moves contents in D into memory location reference by HL
    case 0x72:
      memory[regs->hl] = regs->d;
      break;
    // MOV M,E / 1 byte / 7 cycles /  moves contents in E into memory location reference by HL
    case 0x73:
      memory[regs->hl] = regs->e;
      break;
    // MOV M,H / 1 byte / 7 cycles /  moves contents in H into memory location reference by HL
    case 0x74:
      memory[regs->hl] = regs->h;
      break;
    // MOV M,L / 1 byte / 7 cycles /  moves contents in L into memory location reference by HL
    case 0x75:
      memory[regs->hl] = regs->l;
      break;
    // HLT / 1 byte / 7 cycles / 
    case 0x76:
      break;
    case 0x77:
      break;
    case 0x78:
      break;
    case 0x79:
      break;
    case 0x7A:
      break;
    case 0x7B:
      break;
    case 0x7C:
      break;
    case 0x7D:
      break;
    case 0x7E:
      break;
    case 0x7F:
      break;


    // 80 - 8F ///////////////////////////////////////////////////////
    // ADD B / adds contents of B into A
    case 0x80:
      printf("A = A + B \n");
      break;


    // 90 - 9F ////////////////////////////////////////////////////////
    // SUB B / subtracts the contents of B from A
    case 0x90:
      printf("A = A - B \n");
      break;
     

    // A0 - AF /////////////////////////////////////////////////////////
    // ANA B / bitwize and & between A and B stored in A
    case 0xA0:
      printf("A = A & B. \n");
      break;


    // B0 - BF /////////////////////////////////////////////////////////
    // ORA B / bitwize or | between A and B and stored in A
    case 0xB0:
      printf("A = A | B. \n");
      break;


    // C0 - CF ////////////////////////////////////////////////////////////
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





    // D0 - DF ///////////////////////////////////////////////////////////////
    // RNC / If the carry bit is zero, a return (pop 2 bytes from stack to get) operation is performed.
    case 0xD0:
      printf("If the carry bit is zero, a return operation is performed. \n");
      break;


    // E0 - EF ///////////////////////////////////////////////////////////////
    // RPO / If the Parity bit is zero (indicating odd parity), a return (pop 2 bytes form stack and set pc to it) operation is performed.
    case 0xE0:
      printf("If the Parity bit is zero (indicating odd parity), a return operation is performed. \n");
      break;


    // F0 - FF //////////////////////////////////////////////////////////////
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

int _8080::check_sign_flag(u8 num) {
  int msb = (0x80 & num);
  return (msb == 0x80) ? 1 : 0;
}

int _8080::check_sign_flag(u16 num) {
  int msb = (0x8000 & num);
  return (msb == 0x8000) ? 1 : 0;
}

int _8080::check_zero_flag(int num) {
  return num == 0 ? 1 : 0;
}

int _8080::check_auxilary_flag(u8 num, u8 num2, Operation operation) {
  if (operation == ADD) {
    if (((num & 0x0F) + (num2 & 0x0F)) > 0x0F) {
      return 1;
    }
    else {
      return 0;
    }
  }
  // subtraction 
  else if (operation == SUBTRACT) {
    if ((num2 & 0x0F) > (num & 0x0F)) {
      return 1;
    }
    else {
      return 0;
    }
  }
  // invalid operation
  else {
    return 0;
  }
}

int _8080::check_parity_flag(u16 num) {
  int count = 0;

  while (num > 0) {
    count += num & 0x1;
    num >>= 1;
  }

  return (count % 2 == 0) ? 1 : 0;
}

int check_carry_flag(u8 num, u8 num2, Operation operation) {
  int carry = 0;

  switch (operation) {
    case ADD:
      carry = (num + num2) > 0xFF ? 1 : 0;
      break;
    case SUBTRACT:
      carry = (num >= num2) ? 1 : 0;
      break;
  }

  return carry;
}

int check_carry_flag(u16 num, u16 num2, Operation operation) {
  int carry = 0;

  switch (operation) {
    case ADD:
      carry = (num + num2) > 0xFFFF ? 1 : 0;
      break;
    case SUBTRACT:
      carry = (num >= num2) ? 1 : 0;
      break;
  }

  return carry;
}

