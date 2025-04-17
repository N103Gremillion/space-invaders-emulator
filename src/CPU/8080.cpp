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
  memset(memory, 0, TOTAL_BYTES_OF_MEM);

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
  screen->render_screen(this);
  fill_background();
  draw_instructions();
  regs->render_regs();
}

void _8080::run_test() {
  log_log();

  while (regs->pc != 0x00) {
    if (regs->pc == 0x0005) {
      if (regs->c == 0x02) {
        log_log_nonewl("%c", regs->e);
      } 
      else if (regs->c == 0x09) {
        while (memory[regs->de] != '$') {
          log_log_nonewl("%c", memory[regs->de]);
          regs->de++;
        }
      } 
      else {
        log_error("Unknown c in test interrupt");
        exit(1);
      }
      RET();
    }
    // render();
    u8 opcode = fetch_byte();
    cout << "opcode is 0x" << hex << setw(2) << setfill('0') << static_cast<int>(opcode) << endl;
    // cin.get();
    execute_instruction(opcode);
    SDL_Delay(1);
  }
  log_log();
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
    // cout << "Press any key to continue...";
    // cin.get();

    if (!halted) {
        u8 opcode = fetch_byte();
        execute_instruction(opcode);
        if (regs->pc > INSTRUCTION_CUTTOFF) {
          cout << "Program counter exceeded instruciton cutoff " << regs->pc << " > " << INSTRUCTION_CUTTOFF << "! " << endl;
          while (1) {

          }
        }
    }
  }
  SDL_Delay(0);
}


// use pc to get the next byte in memory
u8 _8080::fetch_byte() {
  u8 opcode = memory[regs->pc];
  regs->pc += 1;
  return opcode;
}

// fetch the next 2 bytes in memory
u16 _8080::fetch_bytes() {
  u8* start = &(memory[regs->pc]);
  u16* bytes = (u16*) (start);
  regs->pc += 2;
  return *bytes;
}

// check type of instruciotn using opcode and perform instruciton
void _8080::execute_instruction(u8 opcode) {
  switch (opcode) {
    // 00 - 0F
    // NOP / 1 byte / 4 cycles / - - - - - /  nothing instruciton
    case 0x00: {cycles += 4; break; }
    // LXI B, d16 / 3 byte / 10 cycles / - - - - - / load preciding 16 bits into register BC
    case 0x01: { LXI_register(&(regs->bc)); cycles += 10; break; }
    // STAX (store accumulator inderectly) B / 1 byte / 7 cycles / - - - - - /  store value of A reg into memory location pointed to by BC reg_pair
    case 0x02: { memory[regs->bc] = regs->a; cycles += 7; break; }
    // INX B / 1 byte / 5 cycles / - - - - - / (increment reg pair) / increment BC reg pair by 1
    case 0x03: { regs->bc++; cycles += 5; break; }
    // INR B / 1 byte / 5 cycles / S Z AC P - /  (incrment reg) / increment B reg by 1
    case 0x04: { increment_register(&(regs->b), &(regs->f)); cycles += 5; break; }
    // DCR B / 1 byte / 5 cycles / S Z AC P - / (decrement reg) / decrement B reg by 1
    case 0x05: { decrement_register(&(regs->b), &(regs->f)); cycles += 5; break; }
    // MVI B, d8 (move immediate) / 2 byte / 7 cycle / - - - - - / move d8 value into B reg
    case 0x06: { regs->b = fetch_byte(); cycles += 7; break; }
    // RLC / 1 byte / 4 cycles / - - - - C / (Rotate left through carry) / shift bits of A by 1 (A << 1) then set LSB (least sig bit) of A to value in carry finally take the MSB (most sig bit) of A and make carry that value
    case 0x07: { 
      regs->ca = ((regs->a & 0x80) == 0x80) ? 1 : 0;
      regs->a = (regs->a << 1) | regs->ca; 
      cycles += 4; 
      break; 
    }
    // NOP / 1 byte / 4 cycles / nothing
    case 0x08: { cycles += 4; break; }
    // DAD B / 1 byte / 10 cycles / - - - - CA / (double add) / add value in BC reg pair to HL reg pair (modifies the carry flag if there is overflow)
    case 0x09: { DAD_register(&(regs->hl), regs->bc, &(regs->f)); cycles += 10; break; }
    // LDAX B / 1 byte / 7 cycles / (load accumulator from mem) / load memory address pointed to by BC (memory[BC]) into A reg 
    case 0x0A: { regs->a = memory[regs->bc]; cycles += 7; break; }
    // DCX B / 1 byte / 5 cyles / - - - - - / decrement BC
    case 0x0B: { regs->bc--; cycles += 5; break; }
    // INC C / 1 byte / 5 cycles / S Z A P - / incremtent c by 1 
    case 0x0C: { increment_register(&(regs->c), &(regs->f)); cycles += 5; break; }
    // DCR C / 1 byte / 5 cycles / S Z AC P - / decrement c by 1
    case 0x0D: { decrement_register(&(regs->c), &(regs->f)); cycles += 5; break; }
    // MVI, C, d8 / 2 bytes / 7 cycles / - - - - - / move next byte into C reg
    case 0x0E: { regs->c = fetch_byte(); cycles += 7; break; }
    // RRC / 1 byte / 4 cycles / - - - - CA / rotate accumulator right
    case 0x0F: {
      int low_bit = (regs->a & 0x01);
      regs->ca = low_bit;
      regs->a = (regs->a >> 1);
      regs->a = (regs->a | (low_bit << 7));
      cycles += 4;
      break;
    }


    // 10 - 1F ///////////////////////////////////////////////////
    // NOP / 1 byte / 4 cycles / - - - - - /  nothing instruciton
    case 0x10: {cycles += 4; break;}
    // LXI D, d16 / 3 bytes / 10 cycles / - - - - - / load the next 2 bytes in memory into reg-pair DE
    case 0x11: { LXI_register(&(regs->de)); cycles += 10; break;}
    // STAX D / 1 byte / 7 cycles / - - - - - / contents of A are stroed in memory reference by the location in DE reg-pair
    case 0x12: {memory[regs->de] = regs->a; cycles += 7; break; }
    // INX D / 1 byte / 5 cycles / - - - - - / DE ++
    case 0x13: {regs->de++; cycles += 5; break;}
    // INR D / 1 byte / 5 cycles / S Z AC P - /  (incrment reg) / increment D reg by 1
    case 0x14: {increment_register(&(regs->d), &(regs->f)); cycles += 5; break;};
    // DCR D / 1 byte / 5 cycles / S Z AC P - / (decrement reg) / decrement D reg by 1
    case 0x15: { decrement_register(&(regs->d), &(regs->f)); cycles += 5; break; };
    // MVI D, d8 (move immediate) / 2 byte / 7 cycle / - - - - - / move d8 value into D reg
    case 0x16: { regs->d = fetch_byte(); cycles += 7; break; }
    // RAL / 1 byte / 4 cycles / - - - - C / A is rotated << 1 and the high bit replaces the carry bit while carry replaces the high bit
    case 0x17: { 
      int cur_carry = regs->ca; 
      regs->ca = (regs->a & 0x80) >> 7; 
      regs->a = (regs->a << 1) | cur_carry; 
      cycles += 4; 
      break; 
    }
    // NOP / 1 byte / 4 cycles / nothing
    case 0x18: { cycles += 4; break; }
    // DAD D / 1 byte / 10 cycles / - - - - CA / (double add) / add value in DE reg pair to HL reg pair (modifies the carry flag if there is overflow)
    case 0x19: { DAD_register(&(regs->hl), regs->de, &(regs->f)); cycles += 10; break; }
    // LDAX D / 1 byte / 7 cycles / (load accumulator from mem) / load memory address pointed to by DE (memory[DE]) into A reg 
    case 0x1A: { regs->a = memory[regs->de]; cycles += 7; break; }
    // DCX D / 1 byte / 5 cyles / - - - - - / decrement DE
    case 0x1B: { regs->de--; cycles += 5; break; }
    // INC E / 1 byte / 5 cycles / S Z A P - / incremtent e by 1 
    case 0x1C: { increment_register(&(regs->e), &(regs->f)); cycles += 5; break; }
    // DCR E / 1 byte / 5 cycles / S Z AC P - / decrement e by 1
    case 0x1D: { decrement_register(&(regs->e), &(regs->f)); cycles += 5; break; }
    // MVI, E, d8 / 2 bytes / 7 cycles / - - - - - / move next byte into E reg
    case 0x1E: { regs->e = fetch_byte(); cycles += 7; break; }
    // RAR / 1 byte / 4 cycles / - - - - CA / rotate accumulator right
    case 0x1F: {
      int prev_carry = regs->ca;
      regs->ca = (regs->a & 0x01);
      regs->a = (regs->a >> 1);
    }

    case 0x21: { LXI_register(&(regs->hl)); cycles += 10; break; }
    // SHLD a16 / 3 bytes / 16 cycles / - - - - - /  memory location referenced by next 2 bytes is set to L and the next memory location after is set to H
    case 0x22: { u16 address = fetch_bytes(); memory[address] = regs->l; memory[address + 1] = regs->h; cycles += 16; break; }
    // INX H / 1 byte / 5 cycles / - - - - - / HL ++
    case 0x23: { regs->hl++; cycles += 5; break; }
    // INR H / 1 byte / 5 cycles / S Z AC P - /  (incrment reg) / increment H reg by 1
    case 0x24: {increment_register(&(regs->h), &(regs->f)); cycles += 5; break;};
    // DCR H / 1 byte / 5 cycles / S Z AC P - / (decrement reg) / decrement H reg by 1
    case 0x25: { decrement_register(&(regs->h), &(regs->f)); cycles += 5; break; }
    // MVI H, d8 (move immediate) / 2 byte / 7 cycle / - - - - - / move d8 value into H reg
    case 0x26: { regs->h = fetch_byte(); cycles += 7; break; }
    // DAA / 1 byte / 4 cycle / S Z AC P CA / (decimal adjust accumulator) 
    case 0x27: {
      u8 ls_4bits = (regs->a & 0x0F);
      if ((ls_4bits > 9) || (regs->ac == 1)) {
        regs->ac = 1;
        regs->a += 0X06;
      } else {
        regs->ac = 0;
      }
      u8 ms_4bits = ((regs->a & 0xF0) >> 4);
      if ((ms_4bits > 0x9) || (regs->ca == 1)) {
        regs->ca = 1;
        regs->a += 0x60;
      } else {
        regs->ca = 0;
      }
      regs->s = check_sign_flag(regs->a);
      regs->z = check_zero_flag(regs->a);
      regs->p = check_parity_flag(regs->a);
      cycles += 4;
      break;
    }
    // NOP / 1 byte / 4 cycles / nothing
    case 0x28: { cycles += 4; break; }
    // DAD H / 1 byte / 10 cycles / - - - - CA / (double add) / add value in HL reg pair to HL reg pair (modifies the carry flag if there is overflow)
    case 0x29: { DAD_register(&(regs->hl), regs->hl, &(regs->f)); cycles += 10; break; }
    // LHLD a16, / 3 byte / 16 cycles / takes 16 bit address and loads content of memory into HL
    case 0x2A: {
      u16 address = fetch_bytes();
      regs->l = memory[address];
      regs->h = memory[address + 1];
      cycles += 16;
      break;
    }
    // DCX H / 1 byte / 5 cyles / - - - - - / decrement HL
    case 0x2B: { regs->hl--; cycles += 5; break; }
    // INC L / 1 byte / 5 cycles / S Z A P - / incremtent L by 1 
    case 0x2C: { increment_register(&(regs->l), &(regs->f)); cycles += 5; break; }
    // DCR L / 1 byte / 5 cycles / S Z AC P - / decrement l by 1
    case 0x2D: { decrement_register(&(regs->l), &(regs->f)); cycles += 5; break; }
    // MVI, L, d8 / 2 bytes / 7 cycles / - - - - - / move next byte into l reg
    case 0x2E: { regs->l = fetch_byte(); cycles += 7; break; }
    // CMA / 1 byte / 4 cycles / - - - - - / complement accumulator
    case 0x2F:
      regs->a = ~regs->a;
      cycles += 4;
      break;

    // 30 - 3F ////////////////////////////////////////////////////
    // NOP / 1 byte / 4 cycles / - - - - - /  nothing instruciton
    case 0x30:{cycles += 4; break; }
    // LXI SP, d16 / 3 bytes / 10 cycles / - - - - - / SP = (next 2 bytes)
    case 0x31: { LXI_register(&(regs->sp)); cycles += 10;break;}
    // STA, a16 / 3 bytes / 13 cycles / - - - - - / memory location referenced by next 2 bytes is set to the A reg
    case 0x32: { u16 address = fetch_bytes(); memory[address] = regs->a; cycles += 13; break; }
    // INX SP / 1 byte / 5 cycles / - - - - - / SP ++
    case 0x33: { regs->sp++; cycles += 5; break; }
    // INR M / 1 byte / 10 cycles / S Z AC P - / increment value stored in memory loaction referenced by HL reg_pair
    case 0x34: {increment_register(&(memory[regs->hl]), &(regs->f)); cycles += 10; break;}
    // DCR M / 1 byte / 10 cycles / S Z AC P - / decrement value stored in memory loaction referenced by HL reg_pair
    case 0x35: { decrement_register(&(memory[regs->hl]), &(regs->f)); cycles += 10; break; }
    // MVI M, d8 (move immediate) / 2 byte / 10 cycle / - - - - - / move d8 value into memory with reference in HL
    case 0x36: { memory[regs->hl] = fetch_byte(); cycles += 10; break; }
    // STC / 1 byte / 4 cycle / - - - - CA / carry bit set to 1
    case 0x37: { regs->ca = 1; cycles += 4; break; }
    // NOP / 1 byte / 4 cycles / nothing
    case 0x38: { cycles += 4; break; }
    // DAD SP / 1 byte / 10 cycles / - - - - CA / (double add) / add value in SP reg pair to HL reg pair (modifies the carry flag if there is overflow)
    case 0x39: { DAD_register(&(regs->hl), regs->sp, &(regs->f)); cycles += 10; break; }
    // LDA a16 / 3 bytes / 13 cycles / - - - - - / load the byte in memory loaction refered to by next 2 bytes into a reg
    case 0x3A: { regs->a = memory[fetch_bytes()]; cycles += 13; break; }
    // DCX SP / 1 byte / 5 cyles / - - - - - / decrement SP
    case 0x3B: { regs->sp--; cycles += 5; break; }
    // INC A / 1 byte / 5 cycles / S Z A P - / incremtent A by 1 
    case 0x3C: { increment_register(&(regs->a), &(regs->f)); cycles += 5; break; }
    // DCR A / 1 byte / 5 cycles / S Z AC P - / decrement a by 1
    case 0x3D: { decrement_register(&(regs->a), &(regs->f)); cycles += 5; break; }
    // MVI A, d8 / 2 bytes / 7 cycles / - - - - - / move next byte into a reg
    case 0x3E: { regs->a = fetch_byte(); cycles += 7; break; }
    // CMC / 1 byte / 4 cycles / - - - - CA / flips the cary bit
    case 0x3F: { regs->ca = ~regs->ca; cycles += 4; break; }


    // 40 - 4F ////////////////////////////////////////////////////
    // MOV B,B / 1 byte / 5 cycles / - - - - - / moves B reg into B
    case 0x40: { regs->b = regs->b; cycles += 5; break; }
    // MOV B, C / 1 byte / 5 cycles / - - - - - / moves C reg val int B
    case 0x41: { regs->b = regs->c; cycles += 5; break; }
    // MOV B, D / 1 byte / 5 cycles / - - - - - / moves D reg val int B
    case 0x42: { regs->b = regs->d; cycles += 5; break; }
    // MOV B, E / 1 byte / 5 cycles / - - - - - / moves E reg val int B
    case 0x43: { regs->b = regs->e; cycles += 5; break; }
    // MOV B, H / 1 byte / 5 cycles / - - - - - / moves H reg val int B
    case 0x44: { regs->b = regs->h; cycles += 5; break; }
    // MOV B, L / 1 byte / 5 cycles / - - - - - / moves L reg val int B
    case 0x45: { regs->b = regs->l; cycles += 5; break; }
    // MOV B, M / 1 byte / 7 cycles / - - - - - / moves value form mem locatioin pointed to by HL into B
    case 0x46: { regs->b = memory[regs->hl]; cycles += 7; break; }
    // MOV B, A / 1 byte / 5 cycles / - - - - - / moves A reg val into B
    case 0x47: { regs->b = regs->a; cycles += 5; break; }
    // MOV C, B / 1 byte / 5 cycles / - - - - - / moves B reg val into C
    case 0x48: { regs->c = regs->b; cycles += 5; break; }
    // MOV C, C / 1 byte / 5 cycles / - - - - - / moves C reg val into C
    case 0x49: { regs->c = regs->c; cycles += 5; break; }
    // MOV C, D / 1 byte / 5 cycles / - - - - - / moves D reg val into C
    case 0x4A: { regs->c = regs->d; cycles += 5; break; }
    // MOV C, E / 1 byte / 5 cycles / - - - - - / moves E reg val into C
    case 0x4B: { regs->c = regs->e; cycles += 5; break; }
    // MOV C, H / 1 byte / 5 cycles / - - - - - / moves H reg val into C
    case 0x4C: { regs->c = regs->h; cycles += 5; break; }
    // MOV C, L / 1 byte / 5 cycles / - - - - - / moves L reg val into C
    case 0x4D: { regs->c = regs->l; cycles += 5; break; }
    // MOV C, M / 1 byte / 7 cycles / - - - - - / moves value in memory location pointed to by HL reg val into C
    case 0x4E: { regs->c = memory[regs->hl]; cycles += 7; break; }
    // MOV C, A / 1 byte / 5 cycles / - - - - - / moves A reg val into C
    case 0x4F: { regs->c = regs->a; cycles += 5; break; }


    // 50 - 5F ////////////////////////////////////////////////////
    // MOV D,B / 1 byte / 5 cycles /  moves B into D
    case 0x50: { regs->d = regs->b; cycles += 5; break; }
    // MOV D, C /  1 byte / 5 cycles / moves C into D
    case 0x51: { regs->d = regs->c; cycles += 5; break; }
    // MOV D, D /  1 byte / 5 cycles / moves D into D
    case 0x52: { regs->d = regs->d; cycles += 5; break; }
    // MOV D, E /  1 byte / 5 cycles / moves E into D
    case 0x53: { regs->d = regs->e; cycles += 5; break; }
    // MOV D, H /  1 byte / 5 cycles / moves H into D
    case 0x54: { regs->d = regs->h; cycles += 5; break; }
    // MOV D, L /  1 byte / 5 cycles / moves L into D
    case 0x55: { regs->d = regs->l; cycles += 5; break; }
    // MOV D, M /  1 byte / 7 cycles / moves contents in memory location spcified by HL into D reg
    case 0x56: { regs->d = memory[regs->hl]; cycles += 7; break; }
    // MOV D, A /  1 byte / 5 cycles / moves A into D
    case 0x57: { regs->d = regs->a; cycles += 5; break; }
    // MOV E, B / 1 byte / 5 cycles / moves B into E
    case 0x58: { regs->e = regs->b; cycles += 5; break; }
    // MOV E, C / 1 byte / 5 cycles / moves C into E
    case 0x59: { regs->e = regs->c; cycles += 5; break; }
    // MOV E, D / 1 byte / 5 cycles / moves D into E
    case 0x5A: { regs->e = regs->d; cycles += 5; break; }
    // MOV E, E / 1 byte / 5 cycles / moves E into E
    case 0x5B: { regs->e = regs->e; cycles += 5; break; }
    // MOV E, H / 1 byte / 5 cycles / moves H into E
    case 0x5C: { regs->e = regs->h; cycles += 5; break; }
    // MOV E, L / 1 byte / 5 cycles / moves L into E
    case 0x5D: { regs->e = regs->l; cycles += 5; break; }
    // MOV E, M / 1 byte / 7 cycles / moves contents in memory location refered to by HL into E
    case 0x5E: { regs->e = memory[regs->hl]; cycles += 7; break; }
    // MOV E, A / 1 byte / 5 cycles / moves the contents of A into E
    case 0x5F: { regs->e = regs->a; cycles += 5; break; }

    // 60 - 6F ////////////////////////////////////////////////////
    // MOV H,B / 1 byte / 5 cycles / moves B into H
    case 0x60: { regs->h = regs->b; cycles += 5; break; }
    // MOV H,C / 1 byte / 5 cycles / moves C into H
    case 0x61: { regs->h = regs->c; cycles += 5; break; }
    // MOV H,D / 1 byte / 5 cycles / moves D into H
    case 0x62: { regs->h = regs->d; cycles += 5; break; }
    // MOV H,E / 1 byte / 5 cycles / moves E into H
    case 0x63: { regs->h = regs->e; cycles += 5; break; }
    // MOV H,H / 1 byte / 5 cycles / moves H into H
    case 0x64: { regs->h = regs->h; cycles += 5; break; }
    // MOV H,L / 1 byte / 5 cycles / moves L into H
    case 0x65: { regs->h = regs->l; cycles += 5; break; }
    // MOV H,M / 1 byte / 7 cycles / moves the value in memory reference by the value in reg HL and sets it to H
    case 0x66: { regs->h = memory[regs->hl]; cycles += 7; break; }
    // MOV H,A / 1 byte / 5 cycles / moves A into H
    case 0x67: { regs->h = regs->a; cycles += 5; break; }
    // MOV L,B / 1 byte / 5 cycles / moves B into L
    case 0x68: { regs->l = regs->b; cycles += 5; break; }
    // MOV L,C / 1 byte / 5 cycles / moves C into L
    case 0x69: { regs->l = regs->c; cycles += 5; break; }
    // MOV L,D / 1 byte / 5 cycles / moves D into L
    case 0x6A: { regs->l = regs->d; cycles += 5; break; }
    // MOV L,E / 1 byte / 5 cycles / moves E into L
    case 0x6B: { regs->l = regs->e; cycles += 5; break; }
    // MOV L,H / 1 byte / 5 cycles / moves H into L
    case 0x6C: { regs->l = regs->h; cycles += 5; break; }
    // MOV L,L / 1 byte / 5 cycles / moves L into L
    case 0x6D: { regs->l = regs->l; cycles += 5; break; }
    // MOV L,M / 1 byte / 7 cylces / moves the value in memory referenced by HL into the L reg
    case 0x6E: { regs->l = memory[regs->hl]; cycles += 7; break; }
    // MOV L,A / 1 byte / 5 cycles / moves A into L
    case 0x6F: { regs->l = regs->a; cycles += 5; break; }


    // 70 - 7F /////////////////////////////////////////////////////
    // MOV M,B / 1 byte / 7 cycles /  moves contents in B into memory location reference by HL
    case 0x70: { memory[regs->hl] = regs->b; cycles += 7; break; }
    // MOV M,C / 1 byte / 7 cycles /  moves contents in C into memory location reference by HL
    case 0x71: { memory[regs->hl] = regs->c; cycles += 7; break; }
    // MOV M,D / 1 byte / 7 cycles /  moves contents in D into memory location reference by HL
    case 0x72: { memory[regs->hl] = regs->d; cycles += 7; break; }
    // MOV M,E / 1 byte / 7 cycles /  moves contents in E into memory location reference by HL
    case 0x73: { memory[regs->hl] = regs->e; cycles += 7; break; }
    // MOV M,H / 1 byte / 7 cycles /  moves contents in H into memory location reference by HL
    case 0x74: { memory[regs->hl] = regs->h; cycles += 7; break; }
    // MOV M,L / 1 byte / 7 cycles /  moves contents in L into memory location reference by HL
    case 0x75: { memory[regs->hl] = regs->l; cycles += 7; break; }
    // HLT / 1 byte / 7 cycles / halts until an interupt occurs
    case 0x76: {
      halted = true;  
      cycles += 7;
      break;
    }
    // MOV M,A / 1 byte / 7 cycles /  moves contents in A into memory location reference by HL
    case 0x77: { memory[regs->hl] = regs->a; cycles += 7; break; }
    // MOV A,B / 1 byte / 5 cycles / moves B contents into A
    case 0x78: { regs->a = regs->b; cycles += 5; break; }
    // MOV A,C / 1 byte / 5 cycles / moves C contents into A
    case 0x79: { regs->a = regs->c; cycles += 5; break; }
    // MOV A,D / 1 byte / 5 cycles / moves D contents into A
    case 0x7A: { regs->a = regs->d; cycles += 5; break; }
    // MOV A,E / 1 byte / 5 cycles / moves E contents into A
    case 0x7B: { regs->a = regs->e; cycles += 5; break; }
    // MOV A,H / 1 byte / 5 cylcles / moves contents of H into A
    case 0x7C: { regs->a = regs->h; cycles += 5; break; }
    // MOV A,L / 1 byte / 5 cyles / moves contents of L into A
    case 0x7D: { regs->a = regs->l; cycles += 5; break; }
    // MOV A,M / 1 byte / 7 cyles / moves contents memory[HL] into A
    case 0x7E: { regs->a = memory[regs->hl]; cycles += 7; break; }
    // MOV A,A / 1 byte / 5 cyles / moves contents of A into A
    case 0x7F: { regs->a = regs->a; cycles += 5; break; }


    // 80 - 8F ///////////////////////////////////////////////////////
    // ADD B / 1 byte / 4  cycles/ S Z AC P CA / adds contents of B into A
    case 0x80: { add_register(&(regs->a), regs->b, &(regs->f)); cycles += 4; break; }
    case 0x81: { add_register(&(regs->a), regs->c, &(regs->f)); cycles += 4; break; }
    case 0x82: { add_register(&(regs->a), regs->d, &(regs->f)); cycles += 4; break; }
    case 0x83: { add_register(&(regs->a), regs->e, &(regs->f)); cycles += 4; break; }
    case 0x84: { add_register(&(regs->a), regs->h, &(regs->f)); cycles += 4; break; }
    case 0x85: { add_register(&(regs->a), regs->l, &(regs->f)); cycles += 4; break; }
    case 0x86: { add_register(&(regs->a), memory[regs->hl], &(regs->f)); cycles += 7; break; }
    case 0x87: { add_register(&(regs->a), regs->a, &(regs->f)); cycles += 4; break; }

    // ADC B / 1 byte / 4 cycles / S Z AC P CA / B and carry are added and stored in A
    case 0x88: { add_register(&(regs->a), (regs->b + (regs->ca ? 1 : 0)), &(regs->f)); cycles += 4; break; }
    case 0x89: { add_register(&(regs->a), (regs->c + (regs->ca ? 1 : 0)), &(regs->f)); cycles += 4; break; }
    case 0x8A: { add_register(&(regs->a), (regs->d + (regs->ca ? 1 : 0)), &(regs->f)); cycles += 4; break; }
    case 0x8B: { add_register(&(regs->a), (regs->e + (regs->ca ? 1 : 0)), &(regs->f)); cycles += 4; break; }
    case 0x8C: { add_register(&(regs->a), (regs->h + (regs->ca ? 1 : 0)), &(regs->f)); cycles += 4; break; }
    case 0x8D: { add_register(&(regs->a), (regs->l + (regs->ca ? 1 : 0)), &(regs->f)); cycles += 4; break; }
    case 0x8E: { add_register(&(regs->a), (memory[regs->hl] + (regs->ca ? 1 : 0)), &(regs->f)); cycles += 7; break; }
    case 0x8F: { add_register(&(regs->a), (regs->a + (regs->ca ? 1 : 0)), &(regs->f)); cycles += 4; break; }


    // 90 - 9F ////////////////////////////////////////////////////////
    // SUB B / 1 byte / 4 cycles / S Z AC P CA / subtracts the contents of B from A and store in A
    case 0x90: { subtract_register(&(regs->a), regs->b, &(regs->f)); cycles +=4; break; }
    case 0x91: { subtract_register(&(regs->a), regs->c, &(regs->f)); cycles +=4; break; }
    case 0x92: { subtract_register(&(regs->a), regs->d, &(regs->f)); cycles +=4; break; }
    case 0x93: { subtract_register(&(regs->a), regs->e, &(regs->f)); cycles +=4; break; }
    case 0x94: { subtract_register(&(regs->h), regs->c, &(regs->f)); cycles +=4; break; }
    case 0x95: { subtract_register(&(regs->a), regs->l, &(regs->f)); cycles +=4; break; }
    case 0x96: {subtract_register(&(regs->a), memory[regs->hl], &(regs->f)); cycles +=7; break; }
    case 0x97: { subtract_register(&(regs->a), regs->a, &(regs->f)); cycles +=4; break; }

    // SBB B / 1 byte / 4 cycles / S Z AC P CA / subtracts the contents of B and CA from A and store in A
    case 0x98: { subtract_register(&(regs->a), (regs->b - (regs->ca ? 1 : 0)), &(regs->f)); cycles +=4; break; } 
    case 0x99: { subtract_register(&(regs->a), (regs->c - (regs->ca ? 1 : 0)), &(regs->f)); cycles +=4; break; }
    case 0x9A: { subtract_register(&(regs->a), (regs->d - (regs->ca ? 1 : 0)), &(regs->f)); cycles +=4; break; }
    case 0x9B: { subtract_register(&(regs->a), (regs->e - (regs->ca ? 1 : 0)), &(regs->f)); cycles +=4; break; }
    case 0x9C: { subtract_register(&(regs->a), (regs->h - (regs->ca ? 1 : 0)), &(regs->f)); cycles +=4; break; }
    case 0x9D: { subtract_register(&(regs->a), (regs->l - (regs->ca ? 1 : 0)), &(regs->f)); cycles +=4; break; }
    case 0x9E: { subtract_register(&(regs->a), (memory[regs->hl] - (regs->ca ? 1 : 0)), &(regs->f)); cycles +=7; break; }
    case 0x9F: { subtract_register(&(regs->a), (regs->a - (regs->ca ? 1 : 0)), &(regs->f)); cycles +=4; break; }
     

    // A0 - AF /////////////////////////////////////////////////////////
    // ANA B / 1 byte /  4 cycles / CA Z AC S P /  bitwize and & between A and B stored in A
    case 0xA0: { bitwise_AND_register(&(regs->a), regs->b, &(regs->f)); cycles += 4; break; }
    case 0xA1: { bitwise_AND_register(&(regs->a), regs->c, &(regs->f)); cycles += 4; break; }
    case 0xA2: { bitwise_AND_register(&(regs->a), regs->d, &(regs->f)); cycles += 4; break; }
    case 0xA3: { bitwise_AND_register(&(regs->a), regs->e, &(regs->f)); cycles += 4; break; }
    case 0xA4: { bitwise_AND_register(&(regs->a), regs->h, &(regs->f)); cycles += 4; break; }
    case 0xA5: { bitwise_AND_register(&(regs->a), regs->l, &(regs->f)); cycles += 4; break; }
    case 0xA6: { bitwise_AND_register(&(regs->a), memory[regs->hl], &(regs->f)); cycles += 7; break; }
    case 0xA7: { bitwise_AND_register(&(regs->a), regs->a, &(regs->f)); cycles += 4; break; }

    // XRA (XOR) B / 1 byte / 4 cycles / S Z AC P CA / XOR the A and specified byte and store in A
    case 0XA8: { bitwise_XOR_register(&(regs->a), regs->b, &(regs->f)); cycles += 4; break; }
    case 0XA9: { bitwise_XOR_register(&(regs->a), regs->c, &(regs->f)); cycles += 4; break; }  
    case 0XAA: { bitwise_XOR_register(&(regs->a), regs->d, &(regs->f)); cycles += 4; break; } 
    case 0XAB: { bitwise_XOR_register(&(regs->a), regs->e, &(regs->f)); cycles += 4; break; } 
    case 0XAC: { bitwise_XOR_register(&(regs->a), regs->h, &(regs->f)); cycles += 4; break; } 
    case 0XAD: { bitwise_XOR_register(&(regs->a), regs->l, &(regs->f)); cycles += 4; break; } 
    case 0XAE: { bitwise_XOR_register(&(regs->a), memory[regs->hl], &(regs->f)); cycles += 7; break; } 
    case 0XAF: { bitwise_XOR_register(&(regs->a), regs->a, &(regs->f)); cycles += 4; break; } 

    // B0 - BF /////////////////////////////////////////////////////////
    // ORA B / 1 byte / 4 cycles / S Z AC P CA /  The specified byte is logically ORed bit by bit with the contents of the accumulator. 
    case 0xB0: { bitwise_OR_register(&(regs->a), regs->b, &(regs->f)); cycles += 4; break; }
    case 0xB1: { bitwise_OR_register(&(regs->a), regs->c, &(regs->f)); cycles += 4; break; }
    case 0xB2: { bitwise_OR_register(&(regs->a), regs->d, &(regs->f)); cycles += 4; break; }
    case 0xB3: { bitwise_OR_register(&(regs->a), regs->e, &(regs->f)); cycles += 4; break; }
    case 0xB4: { bitwise_OR_register(&(regs->a), regs->h, &(regs->f)); cycles += 4; break; }
    case 0xB5: { bitwise_OR_register(&(regs->a), regs->l, &(regs->f)); cycles += 4; break; }
    case 0xB6: { bitwise_OR_register(&(regs->a), memory[regs->hl], &(regs->f)); cycles += 7; break; }
    case 0xB7: { bitwise_OR_register(&(regs->a), regs->a, &(regs->f)); cycles += 4; break; }

    // CMP B / 1 byte / 4 cycles / compare specified byte with the accumulator and set flag accourding to result
    case 0xB8: { compare_register(&(regs->a), regs->b, &(regs->f)); cycles +=4; break; }
    case 0xB9: { compare_register(&(regs->a), regs->c, &(regs->f)); cycles +=4; break; }
    case 0xBA: { compare_register(&(regs->a), regs->d, &(regs->f)); cycles +=4; break; }
    case 0xBB: { compare_register(&(regs->a), regs->e, &(regs->f)); cycles +=4; break; }
    case 0xBC: { compare_register(&(regs->a), regs->h, &(regs->f)); cycles +=4; break; }
    case 0xBD: { compare_register(&(regs->a), regs->l, &(regs->f)); cycles +=4; break; }
    case 0xBE: { compare_register(&(regs->a), memory[regs->hl], &(regs->f)); cycles +=7; break; }
    case 0xBF: { compare_register(&(regs->a), regs->a, &(regs->f)); cycles +=4; break; }

    // C0 - CF ////////////////////////////////////////////////////////////
    // RNZ (return if zero) / 1 byte /  checks the zero flag is 0 pop 2 bytes from stack(address) and set the PC to this location 
    case 0xC0: {
      if (regs->z == 0) {
        RET();
        cycles += 11;
      } else {
        cycles += 5;
      }
      break;
    }
    // POP B / 1 byte / 10 cycles / - - - - - / 
    case 0xC1: { pop_register(&(regs->b), &(regs->c)); cycles += 10; break; }
    // JNZ a16 / 3 bytes / 10 cycles / - - - - - / jump if not zero
    case 0xC2: {
      if (regs->z == 0) {
        JMP();
      } else {
        regs->pc += 2;
      }
      cycles += 10;
      break;
    }
    // JMP a16  / 3 bytes / 10 cycles / - - - - - / uncondition jump to the mem address given by next 2 bytes in memory  
    case 0xC3: { JMP(); cycles += 10; break; }
    // CNZ / 3 bytes / 17/11 cycles / - - - - - / Call if not zero
    case 0xC4: {
      if (regs->z == 0) {
        CALL(fetch_bytes());
        cycles += 17;
      } else {
        fetch_bytes();
        cycles += 11;
      }
      break;
    }
    // PUSH B / 1 byte / 11 cycles / pushes the BC pair onto the stack
    case 0xC5: { push_register(&(regs->b), &(regs->c)); cycles += 11; break; }
    // ADI d8  / 2 bytes / 7 cycles / S AC Z P CA / add immediate to accumulator
    case 0xC6: { add_register(&(regs->a), fetch_byte(), &(regs->f)); cycles += 7; break; } 
    // RST 0 / 1 byte / 11 cycles / jump to n * 8 memory adrees a push pc to the stack
    case 0xC7: { RST(0); cycles += 11; break; }
    // RZ / 1 byte / 11/5 cycles / return if zero
    case 0xC8: {
      if (regs->z == 1) {
        RET();
        cycles += 11;
      } else {
        cycles += 5;
      }
      break;
    }
    // RET / 1 byte / 10 cycles
    case 0xC9: { RET(); cycles += 10; break; }
    // JZ a16 / 3 bytes / 10 cycles / - - - - - / jump if zero
    case 0xCA: {
      if (regs->z == 1) {
        JMP(); 
        cycles += 10; 
      } else {
        fetch_bytes();
        cycles += 7;
      }
      break;
    }
    // JMP a16  / 3 bytes / 10 cycles / - - - - - / uncondition jump to the mem address given by next 2 bytes in memory  
    case 0xCB: { JMP(); cycles += 10; break;}
    // CZ a16 / 3 byte / 17/11 / call if zero
    case 0xCC: {
      if (regs->z == 1) {
        CALL(fetch_bytes());
        cycles += 17;
      } else {
        regs->pc += 2;
        cycles += 11;
      }
      break;
    }
    // CALL / 3 bytes / 17 cycles / 
    case 0xCD: { CALL(fetch_bytes()); cycles += 17; break; }
    // ACI / 2 byte / 7 cyles / add next byte to A and the carry
    case 0xCE: { add_register(&(regs->a), (fetch_byte() + regs->ca), &(regs->f)); cycles += 7; break;}
    // RST 1 / 1 byte / 11 cycles / 
    case 0xCF: { RST(1); cycles += 11; break; }

    // D0 - DF ///////////////////////////////////////////////////////////////
    // RNC (return if no carry) / 1 byte / 11/5 cyles
    case 0xD0: {
      if (regs->ca == 0) {
        RET();
        cycles += 11;
      } else {
        cycles += 5;
      }
      break;
    }
    // POP D / 1 byte / 10 cycles / - - - - - / 
    case 0xD1: { pop_register(&(regs->d), &(regs->e)); cycles += 10; break; }
    // JNC a16 / 3 bytes / 10 cycles / - - - - - / jump if not carry
    case 0xD2: {
      if (regs->ca == 0) {
        JMP();
      } else {
        regs->pc += 2;
      }
      cycles += 10;
      break;
    }
    // OUT d8 / 2 bytes / 10 cycles / 
    case 0XD3: { 
      u8 port = fetch_byte();    
      handle_io(port, OUT, &(regs->a));   
      cycles += 10;
      break;
    }
    // CNC / 3 bytes / 17/11 cycles / - - - - - / Call if not carry
    case 0xD4: {
      if (regs->ca == 0) {
        CALL(fetch_bytes());
        cycles += 17;
      } else {
        fetch_bytes();
        cycles += 11;
      }
      break;
    }
    // PUSH D / 1 byte / 11 cycles / pushes the DE pair onto the stack
    case 0xD5: { push_register(&(regs->d), &(regs->e)); cycles += 11; break; }
    // SUI d8  / 2 bytes / 7 cycles / S AC Z P CA / subtract immediate to accumulator
    case 0xD6: { subtract_register(&(regs->a), fetch_byte(), &(regs->f)); cycles += 7; break; } 
    // RST 2 / 1 byte / 11 cycles / jump to n * 8 memory adrees a push pc to the stack
    case 0xD7: { RST(2); cycles += 11; break; }
    // RC / 1 byte / 11/5 cycles / return if carry
    case 0xD8: {
      if (regs->ca == 1) {
        RET();
        cycles += 11;
      } else {
        cycles += 5;
      }
      break;
    }
    // RET / 1 byte / 10 cycles
    case 0xD9: { RET(); cycles += 10; break; }
    // JC a16 / 3 bytes / 10 cycles / - - - - - / jump if carry
    case 0xDA: {
      if (regs->ca == 1) {
        JMP(); 
        cycles += 10; 
      } else {
        fetch_bytes();
        cycles += 7;
      }
      break;
    }
    // IN d8 / 2 bytes / 10 cycles / 
    case 0XDB: { 
      u8 port = fetch_byte();    
      handle_io(port, IN, &(regs->a));
      cycles += 10;
      break;
    }
    // CC / 3 bytes / 17/11 cyles / call if carry
    case 0xDC : { 
      if (regs->ca){
        CALL(fetch_bytes());
        cycles += 17;
      } else {
        regs->pc += 2;
        cycles += 11;
      }
      break;
    }
    // CALL / 3 bytes / 17 cycles / 
    case 0xDD: { CALL(fetch_bytes()); cycles += 17; break; }
    // SBI / 2 byte / 7 cyles / subtract next byte to A and the carry
    case 0xDE: { subtract_register(&(regs->a), (fetch_byte() + regs->ca), &(regs->f)); cycles += 7; break;}
    // RST 3 / 1 byte / 11 cycles / 
    case 0xDF: { RST(3); cycles += 11; break; }

    // E0 - EF ///////////////////////////////////////////////////////////////
    // RPO / 1 byte / 11/5 cycles / If the Parity bit is zero (indicating odd parity), a return (pop 2 bytes form stack and set pc to it) operation is performed.
    case 0xE0: {
      if (regs->p == 0) {
        RET();
        cycles += 11;
      } else {
        cycles += 5;
      }
      break;
    }
    // POP H / 1 byte / 10 cycles / - - - - - / 
    case 0xE1: { pop_register(&(regs->h), &(regs->l)); cycles += 10; break; }
    // JP0 a16 / 3 bytes / 10 cycles / - - - - - / jump if parity odd
    case 0xE2: {
      if (regs->p == 0) {
        JMP();
      } else {
        regs->pc += 2;
      }
      cycles += 10;
      break;
    }
    // XTHL / 1 byte / 18 cycles / - - - - - / The contents of the L register are exchanged with the contents of the memory byte whose address is held in the stack pointer SP. The contents of the H register are exchanged with the contents of the memory byte whose address is one greater than that held in the stack pointer.
    case 0xE3: {
      u8 address1 = memory[regs->sp];
      u8 address2 = memory[regs->sp + 1];
      memory[regs->sp] = regs->l;
      memory[regs->sp + 1] = regs->h;
      regs->l = address1;
      regs->h = address2;
      cycles += 18;
      break;
    }
    // CPO / 3 bytes / 17/11 cycles / - - - - - / Call if parity odd
    case 0xE4: {
      if (regs->p == 0) {
        CALL(fetch_bytes());
        cycles += 17;
      } else {
        fetch_bytes();
        cycles += 11;
      }
      break;
    }
    // PUSH H / 1 byte / 11 cycles / pushes the HL pair onto the stack
    case 0xE5: { push_register(&(regs->h), &(regs->l)); cycles += 11; break; }
    // ANI d8  / 2 bytes / 7 cycles / S AC Z P CA / And Immediate With Accumulator
    case 0xE6: { bitwise_AND_register(&(regs->a), fetch_byte(), &(regs->f)); cycles += 7; break; }
    // RST 4 / 1 byte / 11 cycles / jump to n * 8 memory adrees a push pc to the stack
    case 0xE7: { RST(4); cycles += 11; break; }
    // RPE / 1 byte / 11/5 cycles / return if parity even
    case 0xE8: {
      if (regs->p == 1) {
        RET();
        cycles += 11;
      } else {
        cycles += 5;
      }
      break;
    }
    // PCHL / 1 byte / 5 cycles / The contents of the H register replace the most significant 8 bits of the program counter, and the con- tents of the L register replace the least significant 8 bits of the program counter.
    case 0xE9: {
      regs->pc = (((u16) regs->h << 8) | regs->l);
      cycles += 5;
      break;
    }
    // JPE a16 / 3 bytes / 10 cycles / - - - - - / jump if parity even
    case 0xEA: {
      if (regs->p == 1) {
        JMP(); 
        cycles += 10; 
      } else {
        fetch_bytes();
        cycles += 7;
      }
      break;
    }
    // XCHG / 1 byte / 5 cycles / - - - - - / The 16 bits of data held in the Hand L registers are exchanged with the 16 bits of data held in the D and E registers
    case 0xEB: {
      u16 temp_val = regs->hl;
      regs->hl = regs->de;
      regs->de = temp_val;
      cycles += 5;
      break;
    }
    // CPE / 3 bytes / 17/11 cyles / call if parity is even(1)
    case 0xEC : { 
      if (regs->p){
        CALL(fetch_bytes());
        cycles += 17;
      } else {
        regs->pc += 2;
        cycles += 11;
      }
      break;
    }
    // CALL / 3 bytes / 17 cycles / 
    case 0xED: { CALL(fetch_bytes()); cycles += 17; break; }
    // XBI / 2 byte / 7 cyles / xor next byte to A 
    case 0xEE: { bitwise_XOR_register(&(regs->a), fetch_byte(), &(regs->f)); cycles += 7; break;}
    // RST 5 / 1 byte / 11 cycles / 
    case 0xEF: { RST(5); cycles += 11; break; }

    // F0 - FF //////////////////////////////////////////////////////////////
    // RP / 1 byte / 11/5 cycles (if sign bit zero return)
    case 0xF0: {
      if (regs->s == 0) {
        RET();
        cycles += 11;
      } else {
        cycles += 5;
      }
      break;
    }
    // POP PSW / 1 byte / 10 cycles / - - - - - / 
    case 0xF1: { pop_register(&(regs->a), &(regs->f)); cycles += 10; break; }
    // JP a16 / 3 bytes / 10 cycles / - - - - - / jump if positive
    case 0xF2: {
      if (regs->s == 0) {
        JMP();
      } else {
        regs->pc += 2;
      }
      cycles += 10;
      break;
    }
    // DI (dissable interupts)
    case 0xF3: { interrupt_enabled = false; cycles += 4; break; }
    // CP / 3 bytes / 17/11 cycles / - - - - - / Call if plus
    case 0xF4: {
      if (regs->s == 0) {
        CALL(fetch_bytes());
        cycles += 17;
      } else {
        fetch_bytes();
        cycles += 11;
      }
      break;
    }
    // PUSH PSW / 1 byte / 11 cycles / pushes the PSW pair onto the stack
    case 0xF5: { push_register(&(regs->a), &(regs->f)); cycles += 11; break; }
    // ORI d8  / 2 bytes / 7 cycles / S AC Z P CA / OR Immediate With Accumulator
    case 0xF6: { bitwise_OR_register(&(regs->a), fetch_byte(), &(regs->f)); cycles += 7; break; }
    // RST 6 / 1 byte / 11 cycles / jump to n * 8 memory adrees a push pc to the stack
    case 0xF7: { RST(6); cycles += 11; break; }
    // RM / 1 byte / 11/5 cycles / return if minus
    case 0xF8: {
      if (regs->s == 1) {
        RET();
        cycles += 11;
      } else {
        cycles += 5;
      }
      break;
    }
    // SPHL / 1 byte / 5 cycles / The 16 bits of data held in the Hand L registers replace the contents of the stack pointer SP.
    case 0xF9: { regs->sp = regs->hl; cycles += 5; break; }
    // JM a16 / 3 bytes / 10 cycles / - - - - - / jump if minus (sign bit is 1)
    case 0xFA: {
      if (regs->s == 1) {
        JMP(); 
        cycles += 10; 
      } else {
        fetch_bytes();
        cycles += 7;
      }
      break;
    }
    //EI (enable interupts) / 4 cycles
    case 0xFB: { interrupt_enabled = true; cycles += 4; break; }
    // CM / 3 bytes / 17/11 cyles / call if minus (sign bit = 1)
    case 0xFC : { 
      if (regs->s){
        CALL(fetch_bytes());
        cycles += 17;
      } else {
        regs->pc += 2;
        cycles += 11;
      }
      break;
    }
    // CALL / 3 bytes / 17 cycles / 
    case 0xFD: { CALL(fetch_bytes()); cycles += 17; break; }
    // CPI / 2 byte / 7 cyles / compare next byte to A 
    case 0xFE: { compare_register(&(regs->a), fetch_byte(), &(regs->f)); cycles += 7; break;}
    // RST 7 / 1 byte / 11 cycles / 
    case 0xFF: { RST(7); cycles += 11; break; }
  }
}

void _8080::LXI_register(u16* reg) {
  u16 bytes = fetch_bytes();    
  *reg = bytes;
}

void _8080::increment_register(u8* reg, u8* flags) {
  u8 res = *(reg) + 1;
  *flags = (*flags & 0xEF) | (check_auxilary_flag(*(reg), res) << AUX_POS); // aux flag
  *flags = (*flags & 0x7F) | (check_sign_flag(res) << SIGN_POS); // sign flag
  *flags = (*flags & 0xBF) | (check_zero_flag(res) << ZERO_POS); // zero flag
  *flags = (*flags & 0xFB) | (check_parity_flag(res) << PARITY_POS); // parity flag
  *reg += 1;
}

void _8080::decrement_register(u8* reg, u8* flags) {
  u8 res = *(reg) - 1;
  *flags = (*flags & 0xEF) | (check_auxilary_flag(*(reg), res) << AUX_POS); // aux flag
  *flags = (*flags & 0x7F) | (check_sign_flag(res) << SIGN_POS); // sign flag
  *flags = (*flags & 0xBF) | (check_zero_flag(res) << ZERO_POS); // zero flag
  *flags = (*flags & 0xFB) | (check_parity_flag(res) << PARITY_POS); // parity flag
  *reg -= 1;
}

void _8080::DAD_register(u16* hl, u16 reg_pair, u8* flags) {
  u16 inital = *hl;
  u32 result = (u32) inital + reg_pair;
  *hl = result;
  *flags = (*flags & 0xFE) | ((check_carry_flag(inital >> 8, result >> 8) << CARRY_POS)); // carry
}

// note: the a is the reg that the result is stored in
void _8080::add_register(u8* a, u8 val, u8* flags) {
  u16 res = *(a) + val;
  *flags = (*flags & 0xFE) | (check_carry_flag(*(a), res) << CARRY_POS); // carry
  *flags = (*flags & 0xEF) | (check_auxilary_flag(*(a), res) << AUX_POS); // aux flag
  *flags = (*flags & 0x7F) | (check_sign_flag(res) << SIGN_POS); // sign flag
  *flags = (*flags & 0xBF) | (check_zero_flag(res) << ZERO_POS); // zero flag
  *flags = (*flags & 0xFB) | (check_parity_flag(res) << PARITY_POS); // parity flag
  *a = res;
}

void _8080::subtract_register(u8* a, u8 val, u8* flags) {
  u16 res = *(a) - val;
  *flags = (*flags & 0xFE) | (check_carry_flag(*(a), res) << CARRY_POS); // carry
  *flags = (*flags & 0xEF) | (check_auxilary_flag(*(a), res) << AUX_POS); // aux flag
  *flags = (*flags & 0x7F) | (check_sign_flag(res) << SIGN_POS); // sign flag
  *flags = (*flags & 0xBF) | (check_zero_flag(res) << ZERO_POS); // zero flag
  *flags = (*flags & 0xFB) | (check_parity_flag(res) << PARITY_POS); // parity flag
  *a = res;
}

void _8080::bitwise_AND_register(u8* a, u8 val, u8* flags) {
  u16 res = *(a) & val;
  *flags = (*flags & 0xFE) | (check_carry_flag(*(a), res) << CARRY_POS); // carry
  *flags = (*flags & 0xEF) | (check_auxilary_flag(*(a), res) << AUX_POS); // aux flag
  *flags = (*flags & 0x7F) | (check_sign_flag(res) << SIGN_POS); // sign flag
  *flags = (*flags & 0xBF) | (check_zero_flag(res) << ZERO_POS); // zero flag
  *flags = (*flags & 0xFB) | (check_parity_flag(res) << PARITY_POS); // parity flag
  *a = res;
}

void _8080::bitwise_XOR_register(u8* a, u8 val, u8* flags) {
  u16 res = *(a) ^ val;
  *flags = (*flags & 0xFE) | (check_carry_flag(*(a), res) << CARRY_POS); // carry
  *flags = (*flags & 0xEF) | (check_auxilary_flag(*(a), res) << AUX_POS); // aux flag
  *flags = (*flags & 0x7F) | (check_sign_flag(res) << SIGN_POS); // sign flag
  *flags = (*flags & 0xBF) | (check_zero_flag(res) << ZERO_POS); // zero flag
  *flags = (*flags & 0xFB) | (check_parity_flag(res) << PARITY_POS); // parity flag
  *a = res;
}

void _8080::bitwise_OR_register(u8* a, u8 val, u8* flags) {
  u16 res = *(a) | val;
  *flags = (*flags & 0xFE) | (check_carry_flag(*(a), res) << CARRY_POS); // carry
  *flags = (*flags & 0xEF) | (check_auxilary_flag(*(a), res) << AUX_POS); // aux flag
  *flags = (*flags & 0x7F) | (check_sign_flag(res) << SIGN_POS); // sign flag
  *flags = (*flags & 0xBF) | (check_zero_flag(res) << ZERO_POS); // zero flag
  *flags = (*flags & 0xFB) | (check_parity_flag(res) << PARITY_POS); // parity flag
  *a = res;
}

void _8080::compare_register(u8* a, u8 val, u8* flags) {
  // note : comparison is done using subtraction
  u16 res = *(a) - val;
  *flags = (*flags & 0xFE) | (check_carry_flag(*(a), res) << CARRY_POS); // carry
  *flags = (*flags & 0xEF) | (check_auxilary_flag(*(a), res) << AUX_POS); // aux flag
  *flags = (*flags & 0x7F) | (check_sign_flag(res) << SIGN_POS); // sign flag
  *flags = (*flags & 0xBF) | (check_zero_flag(res) << ZERO_POS); // zero flag
  *flags = (*flags & 0xFB) | (check_parity_flag(res) << PARITY_POS); // parity flag
}

u16 _8080::pop_stack() {
  u8* start = &(memory[regs->sp]);
  u16* bytes = (u16*) start;
  regs->sp += 2;
  return *bytes;
}

void _8080::pop_register(u8* first, u8* second) {
  // printf("POP: SP = 0x%04X\n", regs->sp);
  // printf("    -> memory: 0x%02X is, 0x%02X is first\n", memory[regs->sp], memory[regs->sp + 1]);
  *second = memory[regs->sp];
  *first = memory[regs->sp + 1];
  // printf("    -> Popped 0x%02X into second, 0x%02X into first\n", *second, *first);
  regs->sp += 2;
  // printf("    -> SP after POP = 0x%04X\n", regs->sp);
}

void _8080::push_register(u8* first, u8* second) {
  memory[regs->sp - 1] = *first;
  memory[regs->sp - 2] = *second;
  regs->sp -= 2;
}


void _8080::RET() {
  u8 low = memory[regs->sp];
  u8 high = memory[regs->sp + 1];
  u16 return_address = ((high << 8) | low);
  regs->pc = return_address;
  regs->sp += 2;
}

void _8080::CALL(u16 memory_address) {
  regs->sp -= 2;

  u8 ret_low = u8(regs->pc & 0xFF);
  u8 ret_high = u8((regs->pc >> 8) & 0xFF);

  memory[regs->sp] = ret_low;       // Low byte
  memory[regs->sp + 1] = ret_high;  // High byte

  regs->pc = memory_address;
}

void _8080::JMP() {
  u16 mem_loc = fetch_bytes();
  regs->pc = mem_loc;
}

void _8080::RST(u16 n) {
  // save the pc to the stack so it can be retreived later
  interrupt_enabled = false;
  regs->sp -= 2;
  memory[regs->sp + 1] = (regs->pc & 0xFF00) >> 8;
  memory[regs->sp] = regs->pc & 0x00FF;
  regs->pc = n * 8;
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

int _8080::check_auxilary_flag(u8 num, u16 res) {
  if (((num & 0xF) + (res & 0xF)) >> 4 > 0xF) {
    return 1;
  }
  else{
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

int _8080::check_carry_flag(u8 initial, u16 result) {
  // Check if the result indicates a carry has occurred
    if (result > OVERFLOW) { // Check if carry occurred 
        return 1;  // Set the carry flag
    } else {
        return 0;  // Clear the carry flag
    }
}

// int _8080::check_carry_flag(u16 num, u16 num2, Operation operation) {
//   int carry = 0;

//   switch (operation) {
//     case ADD:
//       carry = (num + num2) > 0xFFFF ? 1 : 0;
//       break;
//     case SUBTRACT:
//       carry = (num >= num2) ? 1 : 0;
//       break;
//   }

//   return carry;
// }

uint8_t offset = 0;
typedef union{
    struct {
        uint8_t low_value;
        uint8_t high_value;
    };
    uint16_t value;
} _shift_register;

_shift_register shift_register;

#define SHIFT_AND_BITS 0b00000111

void _8080::handle_io(u8 port_num, PortType type, u8* a) {
  // input ports
  if (type == IN) {
    *a = 0;
    switch (port_num)
    {
      case INP0:
        break;
      case INP1: { 
        #define CREDIT 0
        #define TWOP_START 1
        #define ONEP_START 2
        #define ALWAYS_ONE 3
        #define ONEP_SHOT 4
        #define ONEP_LEFT 5
        #define ONEP_RIGHT 6
        #define NOT_CONNECTED 7

        uint8_t reg_a = 0;  

        if (inputs[INSERT_COIN])
            reg_a |= (1 << CREDIT);       
        else
            reg_a &= ~(1 << CREDIT);     

        reg_a &= ~(1 << TWOP_START);      

        if (inputs[SPACE_KEY])
            reg_a |= (1 << ONEP_START);
        else
            reg_a &= ~(1 << ONEP_START);

        reg_a |= (1 << ALWAYS_ONE);       

        if (inputs[SPACE_KEY])
            reg_a |= (1 << ONEP_SHOT);
        else
            reg_a &= ~(1 << ONEP_SHOT);

        if (inputs[A_KEY])
            reg_a |= (1 << ONEP_LEFT);
        else
            reg_a &= ~(1 << ONEP_LEFT);

        if (inputs[D_KEY])
            reg_a |= (1 << ONEP_RIGHT);
        else
            reg_a &= ~(1 << ONEP_RIGHT);

        reg_a &= ~(1 << NOT_CONNECTED);   

        *a = reg_a; 
        break;
      }
      case INP2:
        break;
      case SHFT_IN: {
        *a = (((shift_register.high_value << 8) | shift_register.low_value) << offset) >> 8;
        break;
      }
    }
  }

  // output ports
  if (type == OUT) {
    u8 value = *a;
    switch (port_num) {
      case SHFTAMNT:
        offset = value & SHIFT_AND_BITS;
        break;
      case SOUND1:
        break;
      case SHFT_DATA:
        shift_register.low_value = shift_register.high_value;
        shift_register.high_value = value;
        break;
      case SOUND2:
        break;
      case WATCHDOG:
        break;
    }
  }
  return; 
}

void _8080::handleCPMCall() {
  switch (regs->c) {
      case 0x02:
          std::cout << static_cast<char>(regs->e);
          break;
      case 0x09: {
          uint16_t addr = (regs->d << 8) | regs->e;
          while (memory[addr] != '$') {
              std::cout << static_cast<char>(memory[addr]);
              addr++;
          }
          break;
      }
      default:
          std::cerr << "Unhandled CP/M call: " << std::hex << int(regs->c) << std::endl;
  }

  // Simulate RET
  regs->pc = (memory[regs->sp + 1] << 8) | memory[regs->sp];
  regs->sp += 2;
}

void _8080::execute_interrupt(int opcode) {
  if (interrupt_enabled) {
    halted = false;           
    execute_instruction(opcode);
  }
}
