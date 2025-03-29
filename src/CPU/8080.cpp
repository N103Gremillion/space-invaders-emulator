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
    // u16 low = fetch_byte();
    // u16 high = fetch_byte();
    // cout << hex << ((high << 8) | low) << endl;
    // SDL_Delay(1000);
  }
}

// use pc to get the next byte in memory
u8 _8080::fetch_byte() {
  u8 opcode = memory[regs->pc];
  regs->pc += 1;
  return opcode;
}

// fetch the next 2 bytes in memory
u16 _8080::fetch_bytes() {
  static int i = 0;
  i++;
  u8* start = &(memory[regs->pc]);
  u16* bytes = (u16*) (start);
  // cout << hex << *(bytes) << endl;
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
      regs->ca = check_carry_flag(regs->a, 0, RLC); 
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
        regs->ac = check_auxilary_flag(regs->a, (regs->a + 6));
        regs->a += 6;
      }
      u8 ms_4bits = (regs->a >> 4);
      if ((ms_4bits > 9) || (regs->ca == 1)) {
        regs->ca = check_carry_flag(regs->a, 0x60, ADD);
        regs->a += 0x60;
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
    case 0x3F:
      regs->ca = ~regs->ca;
      cycles += 4;
      break;


    // 40 - 4F ////////////////////////////////////////////////////
    // MOV B,B / 1 byte / 5 cycles / - - - - - / moves B reg into B
    case 0x40:
      regs->b = regs->b;
      cycles += 5;
      break;
    // MOV B, C / 1 byte / 5 cycles / - - - - - / moves C reg val int B
    case 0x41:
      regs->b = regs->c;
      cycles += 5;
      break;
    // MOV B, D / 1 byte / 5 cycles / - - - - - / moves D reg val int B
    case 0x42:
      regs->b = regs->d;
      cycles += 5;
      break;
    // MOV B, E / 1 byte / 5 cycles / - - - - - / moves E reg val int B
    case 0x43:
      regs->b = regs->e;
      cycles += 5;
      break;
    // MOV B, H / 1 byte / 5 cycles / - - - - - / moves H reg val int B
    case 0x44:
      regs->b = regs->h;
      cycles += 5;
      break;
    // MOV B, L / 1 byte / 5 cycles / - - - - - / moves L reg val int B
    case 0x45:
      regs->b = regs->l;
      cycles += 5;
      break;
    // MOV B, M / 1 byte / 7 cycles / - - - - - / moves value form mem locatioin pointed to by HL into B
    case 0x46:
      regs->b = memory[regs->hl];
      cycles += 7;
      break;
    // MOV B, A / 1 byte / 5 cycles / - - - - - / moves A reg val into B
    case 0x47:
      regs->b = regs->a;
      cycles += 5;
      break;
    // MOV C, B / 1 byte / 5 cycles / - - - - - / moves B reg val into C
    case 0x48:
      regs->c = regs->b;
      cycles += 5;
      break;
    // MOV C, C / 1 byte / 5 cycles / - - - - - / moves C reg val into C
    case 0x49:
      regs->c = regs->c;
      cycles += 5;
      break;
    // MOV C, D / 1 byte / 5 cycles / - - - - - / moves D reg val into C
    case 0x4A:
      regs->c = regs->d;
      cycles += 5;
      break;
    // MOV C, E / 1 byte / 5 cycles / - - - - - / moves E reg val into C
    case 0x4B:
      regs->c = regs->e;
      cycles += 5;
      break;
    // MOV C, H / 1 byte / 5 cycles / - - - - - / moves H reg val into C
    case 0x4C:
      regs->c = regs->h;
      cycles += 5;
      break;
    // MOV C, L / 1 byte / 5 cycles / - - - - - / moves L reg val into C
    case 0x4D:
      regs->c = regs->l;
      cycles += 5;
      break;
    // MOV C, M / 1 byte / 7 cycles / - - - - - / moves value in memory location pointed to by HL reg val into C
    case 0x4E:
      regs->c = memory[regs->hl];
      cycles += 7;
      break;
    // MOV C, A / 1 byte / 5 cycles / - - - - - / moves A reg val into C
    case 0x4F:
      regs->c = regs->a;
      cycles += 5;
      break;


    // 50 - 5F ////////////////////////////////////////////////////
    // MOV D,B / 1 byte / 5 cycles /  moves B into D
    case 0x50:
      regs->d = regs->b;
      cycles += 5;
      break;
    // MOV D, C /  1 byte / 5 cycles / moves C into D
    case 0x51:
      regs->d = regs->c;
      cycles += 5;
      break;
    // MOV D, D /  1 byte / 5 cycles / moves D into D
    case 0x52:
      regs->d = regs->d;
      cycles += 5;
      break;
    // MOV D, E /  1 byte / 5 cycles / moves E into D
    case 0x53:
      regs->d = regs->e;
      cycles += 5;
      break;
    // MOV D, H /  1 byte / 5 cycles / moves H into D
    case 0x54:
      regs->d = regs->h;
      cycles += 5;
      break;
    // MOV D, L /  1 byte / 5 cycles / moves L into D
    case 0x55:
      regs->d = regs->l;
      cycles += 5;
      break;
    // MOV D, M /  1 byte / 7 cycles / moves contents in memory location spcified by HL into D reg
    case 0x56:  
      regs->d = memory[regs->hl];
      cycles += 7;
      break;
    // MOV D, A /  1 byte / 5 cycles / moves A into D
    case 0x57:
      regs->d = regs->a;
      cycles += 5;
      break;
    // MOV E, B / 1 byte / 5 cycles / moves B into E
    case 0x58:
      regs->e = regs->b;
      cycles += 5;
      break;
    // MOV E, C / 1 byte / 5 cycles / moves C into E
    case 0x59:
      regs->e = regs->c;
      cycles += 5;
      break;
    // MOV E, D / 1 byte / 5 cycles / moves D into E
    case 0x5A:
      regs->e = regs->d;
      cycles += 5;
      break;
    // MOV E, E / 1 byte / 5 cycles / moves E into E
    case 0x5B:
      regs->e = regs->e;
      cycles += 5;
      break;
    // MOV E, H / 1 byte / 5 cycles / moves H into E
    case 0x5C:
      regs->e = regs->h;
      cycles += 5;
      break;
    // MOV E, L / 1 byte / 5 cycles / moves L into E
    case 0x5D:
      regs->e = regs->l;
      cycles += 5;
      break;
    // MOV E, M / 1 byte / 7 cycles / moves contents in memory location refered to by HL into E
    case 0x5E:
      regs->e = memory[regs->hl];
      cycles += 7;
      break;
    // MOV E, A / 1 byte / 5 cycles / moves the contents of A into E
    case 0x5F:
      regs->e = regs->a;
      cycles += 5;
      break;

    // 60 - 6F ////////////////////////////////////////////////////
    // MOV H,B / 1 byte / 5 cycles / moves B into H
    case 0x60:
      regs->h = regs->b;
      cycles += 5;
      break;
    // MOV H,C / 1 byte / 5 cycles / moves C into H
    case 0x61:
      regs->h = regs->c;
      cycles += 5;
      break;
    // MOV H,D / 1 byte / 5 cycles / moves D into H
    case 0x62:
      regs->h = regs->d;
      cycles += 5;
      break;
    // MOV H,E / 1 byte / 5 cycles / moves E into H
    case 0x63:
      regs->h = regs->e;
      cycles += 5;
      break;
    // MOV H,H / 1 byte / 5 cycles / moves H into H
    case 0x64:
      regs->h = regs->h;
      cycles += 5;
      break;
    // MOV H,L / 1 byte / 5 cycles / moves L into H
    case 0x65:
      regs->h = regs->l;
      cycles += 5;
      break;
    // MOV H,M / 1 byte / 7 cycles / moves the value in memory reference by the value in reg HL and sets it to H
    case 0x66:
      regs->h = memory[regs->hl];
      cycles += 7;
      break;
    // MOV H,A / 1 byte / 5 cycles / moves A into H
    case 0x67:
      regs->h = regs->a;
      cycles += 5;
      break;
    // MOV L,B / 1 byte / 5 cycles / moves B into L
    case 0x68:
      regs->l = regs->b;
      cycles += 5;
      break;
    // MOV L,C / 1 byte / 5 cycles / moves C into L
    case 0x69:
      regs->l = regs->c;
      cycles += 5;
      break;
    // MOV L,D / 1 byte / 5 cycles / moves D into L
    case 0x6A:
      regs->l = regs->d;
      cycles += 5;
      break;
    // MOV L,E / 1 byte / 5 cycles / moves E into L
    case 0x6B:
      regs->l = regs->e;
      cycles += 5;
      break;
    // MOV L,H / 1 byte / 5 cycles / moves H into L
    case 0x6C:
      regs->l = regs->h;
      cycles += 5;
      break;
    // MOV L,L / 1 byte / 5 cycles / moves L into L
    case 0x6D:
      regs->l = regs->l;
      cycles += 5;
      break;
    // MOV L,M / 1 byte / 7 cylces / moves the value in memory referenced by HL into the L reg
    case 0x6E:
      regs->l = memory[regs->hl];
      cycles += 7;
      break;
    // MOV L,A / 1 byte / 5 cycles / moves A into L
    case 0x6F:
      regs->l = regs->a;
      cycles += 5;
      break;


    // 70 - 7F /////////////////////////////////////////////////////
    // MOV M,B / 1 byte / 7 cycles /  moves contents in B into memory location reference by HL
    case 0x70:
      memory[regs->hl] = regs->b;
      cycles += 7;
      break;
    // MOV M,C / 1 byte / 7 cycles /  moves contents in C into memory location reference by HL
    case 0x71:
      memory[regs->hl] = regs->c;
      cycles += 7;
      break;
    // MOV M,D / 1 byte / 7 cycles /  moves contents in D into memory location reference by HL
    case 0x72:
      memory[regs->hl] = regs->d;
      cycles += 7;
      break;
    // MOV M,E / 1 byte / 7 cycles /  moves contents in E into memory location reference by HL
    case 0x73:
      memory[regs->hl] = regs->e;
      cycles += 7;
      break;
    // MOV M,H / 1 byte / 7 cycles /  moves contents in H into memory location reference by HL
    case 0x74:
      memory[regs->hl] = regs->h;
      cycles += 7;
      break;
    // MOV M,L / 1 byte / 7 cycles /  moves contents in L into memory location reference by HL
    case 0x75:
      memory[regs->hl] = regs->l;
      cycles += 7;
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
    // ADD B / 1 byte / 4  cycles/ S Z AC P CA / adds contents of B into A
    case 0x80:
      add_register(&(regs->a), regs->b, &(regs->f));
      cycles += 4;
      break;
    // ADD C / 1 byte / 4  cycles/ S Z AC P CA / adds contents of C into A
    case 0x81:
      add_register(&(regs->a), regs->c, &(regs->f));
      cycles += 4;
      break;
    // ADD D / 1 byte / 4  cycles/ S Z AC P CA / adds contents of D into A
    case 0x82:
      add_register(&(regs->a), regs->d, &(regs->f));
      cycles += 4;
      break;
    // ADD E / 1 byte / 4  cycles/ S Z AC P CA / adds contents of E into A
    case 0x83:
      add_register(&(regs->a), regs->e, &(regs->f));;
      cycles += 4;
      break;
    // ADD H / 1 byte / 4  cycles/ S Z AC P CA / adds contents of H into A
    case 0x84:
      add_register(&(regs->a), regs->h, &(regs->f));
      cycles += 4;
      break;
    // ADD L / 1 byte / 4  cycles/ S Z AC P CA / adds contents of L into A
    case 0x85:
      add_register(&(regs->a), regs->l, &(regs->f));
      cycles += 4;
      break;
    // ADD M / 1 byte / 7 cycles/ S Z AC P CA / adds contents in memory[hl] into A
    case 0x86:
      add_register(&(regs->a), memory[regs->hl], &(regs->f));
      cycles += 7;
      break;
    // ADD A / 1 byte / 4 cycles/ S Z AC P CA / adds A into A
    case 0x87:
      add_register(&(regs->a), regs->a, &(regs->f));
      cycles += 4;
      break;
    // ADC B / 1 byte / 4 cycles / S Z AC P CA / B and carry are added and stored in A
    case 0x88:
      add_register(&(regs->a), (regs->b + (regs->ca ? 1 : 0)), &(regs->f));
      cycles += 4;
      break;
    // ADC C / 1 byte / 4 cycles / S Z AC P CA / C and carry are added and stored in A
    case 0x89:
      add_register(&(regs->a), (regs->c + (regs->ca ? 1 : 0)), &(regs->f));
      cycles += 4;
      break;
    // ADC D / 1 byte / 4 cycles / S Z AC P CA / D and carry are added and stored in A
    case 0x8A:
      add_register(&(regs->a), (regs->d + (regs->ca ? 1 : 0)), &(regs->f));
      cycles += 4;
      break;
    // ADC E / 1 byte / 4 cycles / S Z AC P CA / E and carry are added and stored in A
    case 0x8B:
      add_register(&(regs->a), (regs->e + (regs->ca ? 1 : 0)), &(regs->f));
      cycles += 4;
      break;
    // ADC H / 1 byte / 4 cycles / S Z AC P CA / H and carry are added and stored in A
    case 0x8C:
      add_register(&(regs->a), (regs->h + (regs->ca ? 1 : 0)), &(regs->f));
      cycles += 4;
      break;
    // ADC L / 1 byte / 4 cycles / S Z AC P CA / L and carry are added and stored in A
    case 0x8D:
      add_register(&(regs->a), (regs->l + (regs->ca ? 1 : 0)), &(regs->f));
      cycles += 4;
      break;
    // ADC M / 1 byte / 7 cycles / S Z AC P CA / memory[HL] and carry are added and stored in A
    case 0x8E:
      add_register(&(regs->a), (memory[regs->hl] + (regs->ca ? 1 : 0)), &(regs->f));
      cycles += 7;
      break;
    // ADC A / 1 byte / 4 cycles / S Z AC P CA / A and carry are added and stored in A
    case 0x8F:
      add_register(&(regs->a), (regs->a + (regs->ca ? 1 : 0)), &(regs->f));
      cycles += 4;
      break;


    // 90 - 9F ////////////////////////////////////////////////////////
    // SUB B / 1 byte / 4 cycles / S Z AC P CA / subtracts the contents of B from A and store in A
    case 0x90:
      subtract_register(&(regs->a), regs->b, &(regs->f));
      cycles +=4;
      break;
    // SUB C / 1 byte / 4 cycles / S Z AC P CA / subtracts the contents of C from A and store in A
    case 0x91:
      subtract_register(&(regs->a), regs->c, &(regs->f));
      cycles +=4;
      break;
    // SUB D / 1 byte / 4 cycles / S Z AC P CA / subtracts the contents of D from A and store in A
    case 0x92:
      subtract_register(&(regs->a), regs->d, &(regs->f));
      cycles +=4;
      break;
    // SUB E / 1 byte / 4 cycles / S Z AC P CA / subtracts the contents of E from A and store in A
    case 0x93:
      subtract_register(&(regs->a), regs->e, &(regs->f));
      cycles +=4;
      break;
    // SUB H / 1 byte / 4 cycles / S Z AC P CA / subtracts the contents of H from A and store in A
    case 0x94:
      subtract_register(&(regs->h), regs->c, &(regs->f));
      cycles +=4;
      break;
    // SUB L / 1 byte / 4 cycles / S Z AC P CA / subtracts the contents of L from A and store in A
    case 0x95:
      subtract_register(&(regs->a), regs->l, &(regs->f));
      cycles +=4;
      break;
    // SUB M / 1 byte / 7 cycles / S Z AC P CA / subtracts memory[HL] from A and store in A
    case 0x96:
      subtract_register(&(regs->a), memory[regs->hl], &(regs->f));
      cycles +=7;
      break;
    // SUB A / 1 byte / 4 cycles / S Z AC P CA / subtracts the contents of A from A and store in A
    case 0x97:
      subtract_register(&(regs->a), regs->a, &(regs->f));
      cycles +=4;
      break;
    // SBB B / 1 byte / 4 cycles / S Z AC P CA / subtracts the contents of B and CA from A and store in A
    case 0x98:
      subtract_register(&(regs->a), (regs->b - (regs->ca ? 1 : 0)), &(regs->f));
      cycles +=4;
      break;
    // SBB C / 1 byte / 4 cycles / S Z AC P CA / subtracts the contents of C and CA from A and store in A
    case 0x99:
      subtract_register(&(regs->a), (regs->c - (regs->ca ? 1 : 0)), &(regs->f));
      cycles +=4;
      break;
    // SBB D / 1 byte / 4 cycles / S Z AC P CA / subtracts the contents of D and CA from A and store in A
    case 0x9A:
      subtract_register(&(regs->a), (regs->d - (regs->ca ? 1 : 0)), &(regs->f));
      cycles +=4;
      break;
    // SBB E / 1 byte / 4 cycles / S Z AC P CA / subtracts the contents of E and CA from A and store in A
    case 0x9B:
      subtract_register(&(regs->a), (regs->e - (regs->ca ? 1 : 0)), &(regs->f));
      cycles +=4;
      break;
    // SBB H / 1 byte / 4 cycles / S Z AC P CA / subtracts the contents of H and CA from A and store in A
    case 0x9C:
      subtract_register(&(regs->a), (regs->h - (regs->ca ? 1 : 0)), &(regs->f));
      cycles +=4;
      break;
    // SBB L / 1 byte / 4 cycles / S Z AC P CA / subtracts the contents of L and CA from A and store in A
    case 0x9D:
      subtract_register(&(regs->a), (regs->l - (regs->ca ? 1 : 0)), &(regs->f));
      cycles +=4;
      break;
    // SBB M / 1 byte / 7 cycles / S Z AC P CA / subtracts memory[HL] and CA from A and store in A
    case 0x9E:
      subtract_register(&(regs->a), (memory[regs->hl] - (regs->ca ? 1 : 0)), &(regs->f));
      cycles +=7;
      break;
    // SBB A / 1 byte / 4 cycles / S Z AC P CA / subtracts the contents of A and CA from A and store in A
    case 0x9F:
      subtract_register(&(regs->a), (regs->a - (regs->ca ? 1 : 0)), &(regs->f));
      cycles +=4;
      break;
     

    // A0 - AF /////////////////////////////////////////////////////////
    // ANA B / 1 byte /  4 cycles / CA Z AC S P /  bitwize and & between A and B stored in A
    case 0xA0:
      bitwise_AND_register(&(regs->a), regs->b, &(regs->f));
      cycles += 4;
      break;
    // ANA C / 1 byte / 4 cycles / CA Z AC S P / bitwize and & between A and C stored in A
    case 0xA1:
      bitwise_AND_register(&(regs->a), regs->c, &(regs->f));
      cycles += 4;
      break;
    // ANA D / 1 byte / 4 cycles / CA Z AC S P / bitwize and & between A and D stored in A
    case 0xA2:
      bitwise_AND_register(&(regs->a), regs->d, &(regs->f));
      cycles += 4;
      break;
    // ANA E / 1 byte / 4 cycles / CA Z AC S P / bitwize and & between A and E stored in A
    case 0xA3:
      bitwise_AND_register(&(regs->a), regs->e, &(regs->f));
      cycles += 4;
      break;
    // ANA H / 1 byte / 4 cycles / CA Z AC S P / bitwize and & between A and H stored in A
    case 0xA4:
      bitwise_AND_register(&(regs->a), regs->h, &(regs->f));
      cycles += 4;
      break;
    // ANA L / 1 byte / 4 cycles / CA Z AC S P / bitwize and & between A and L stored in A
    case 0xA5:
      bitwise_AND_register(&(regs->a), regs->l, &(regs->f));
      cycles += 4;
      break;
    // ANA M / 1 byte / 7 cycles / CA Z AC S P / bitwize and & between A and memory[HL] stored in A
    case 0xA6:
      bitwise_AND_register(&(regs->a), memory[regs->hl], &(regs->f));
      cycles += 7;
      break;
    // ANA A / 1 byte / 4 cycles / CA Z AC S P / bitwize and & between A and A stored in A
    case 0xA7:
      bitwise_AND_register(&(regs->a), regs->a, &(regs->f));
      cycles += 4;
      break;
    // XRA (XOR) B / 1 byte / 4 cycles / S Z AC P CA / XOR the A and specified byte and store in A
    case 0XA8: { bitwise_XOR_register(&(regs->a), regs->b, &(regs->f)); cycles += 4; break; }
    // XRA (XOR) C / 1 byte / 4 cycles / S Z AC P CA / XOR the A and specified byte and store in A
    case 0XA9: { bitwise_XOR_register(&(regs->a), regs->c, &(regs->f)); cycles += 4; break; }  
    // XRA (XOR) D / 1 byte / 4 cycles / S Z AC P CA / XOR the A and specified byte and store in A
    case 0XAA: { bitwise_XOR_register(&(regs->a), regs->d, &(regs->f)); cycles += 4; break; } 
    // XRA (XOR) E / 1 byte / 4 cycles / S Z AC P CA / XOR the A and specified byte and store in A
    case 0XAB: { bitwise_XOR_register(&(regs->a), regs->e, &(regs->f)); cycles += 4; break; } 
    // XRA (XOR) H / 1 byte / 4 cycles / S Z AC P CA / XOR the A and specified byte and store in A
    case 0XAC: { bitwise_XOR_register(&(regs->a), regs->h, &(regs->f)); cycles += 4; break; } 
    // XRA (XOR) L / 1 byte / 4 cycles / S Z AC P CA / XOR the A and specified byte and store in A
    case 0XAD: { bitwise_XOR_register(&(regs->a), regs->l, &(regs->f)); cycles += 4; break; } 
    // XRA (XOR) M / 1 byte / 7 cycles / S Z AC P CA / XOR the A and specified byte and store in A
    case 0XAE: { bitwise_XOR_register(&(regs->a), memory[regs->hl], &(regs->f)); cycles += 7; break; } 
    // XRA (XOR) C / 1 byte / 4 cycles / S Z AC P CA / XOR the A and specified byte and store in A
    case 0XAF: { bitwise_XOR_register(&(regs->a), regs->a, &(regs->f)); cycles += 4; break; } 

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
      u16 mem_loc = fetch_bytes();
      regs->pc = mem_loc;
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
}

void _8080::decrement_register(u8* reg, u8* flags) {
  u8 res = *(reg) - 1;
  *flags = (*flags & 0xEF) | (check_auxilary_flag(*(reg), res) << AUX_POS); // aux flag
  *flags = (*flags & 0x7F) | (check_sign_flag(res) << SIGN_POS); // sign flag
  *flags = (*flags & 0xBF) | (check_zero_flag(res) << ZERO_POS); // zero flag
  *flags = (*flags & 0xFB) | (check_parity_flag(res) << PARITY_POS); // parity flag
}

void _8080::DAD_register(u16* hl, u16 reg_pair, u8* flags) {
  *flags = (*flags & 0xFE) | (check_carry_flag(*(hl), reg_pair, ADD) << CARRY_POS); // carry
  *(hl) = *(hl) + reg_pair;
}

// note: the a is the reg that the result is stored in
void _8080::add_register(u8* a, u8 val, u8* flags) {
  u8 res = *(a) + val;
  *flags = (*flags & 0xFE) | (check_carry_flag(*(a), val, ADD) << CARRY_POS); // carry
  *flags = (*flags & 0xEF) | (check_auxilary_flag(*(a), res) << AUX_POS); // aux flag
  *flags = (*flags & 0x7F) | (check_sign_flag(res) << SIGN_POS); // sign flag
  *flags = (*flags & 0xBF) | (check_zero_flag(res) << ZERO_POS); // zero flag
  *flags = (*flags & 0xFB) | (check_parity_flag(res) << PARITY_POS); // parity flag
  *a = res;
}

void _8080::subtract_register(u8* a, u8 val, u8* flags) {
  u8 res = *(a) - val;
  *flags = (*flags & 0xFE) | (check_carry_flag(*(a), val, SUBTRACT) << CARRY_POS); // carry
  *flags = (*flags & 0xEF) | (check_auxilary_flag(*(a), res) << AUX_POS); // aux flag
  *flags = (*flags & 0x7F) | (check_sign_flag(res) << SIGN_POS); // sign flag
  *flags = (*flags & 0xBF) | (check_zero_flag(res) << ZERO_POS); // zero flag
  *flags = (*flags & 0xFB) | (check_parity_flag(res) << PARITY_POS); // parity flag
  *a = res;
}

void _8080::bitwise_AND_register(u8* a, u8 val, u8* flags) {
  u8 res = *(a) & val;
  *flags = (*flags & 0xFE) | (check_carry_flag(*(a), val, AND) << CARRY_POS); // carry
  *flags = (*flags & 0xEF) | (check_auxilary_flag(*(a), res) << AUX_POS); // aux flag
  *flags = (*flags & 0x7F) | (check_sign_flag(res) << SIGN_POS); // sign flag
  *flags = (*flags & 0xBF) | (check_zero_flag(res) << ZERO_POS); // zero flag
  *flags = (*flags & 0xFB) | (check_parity_flag(res) << PARITY_POS); // parity flag
  *a = res;
}

void _8080::bitwise_XOR_register(u8* a, u8 val, u8* flags) {
  u8 res = *(a) ^ val;
  *flags = (*flags & 0xFE) | (check_carry_flag(*(a), val, XOR) << CARRY_POS); // carry
  *flags = (*flags & 0xEF) | (check_auxilary_flag(*(a), res) << AUX_POS); // aux flag
  *flags = (*flags & 0x7F) | (check_sign_flag(res) << SIGN_POS); // sign flag
  *flags = (*flags & 0xBF) | (check_zero_flag(res) << ZERO_POS); // zero flag
  *flags = (*flags & 0xFB) | (check_parity_flag(res) << PARITY_POS); // parity flag
  *a = res;
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

int _8080::check_carry_flag(u8 num, u8 num2, Operation operation) {
  int carry = 0;

  switch (operation) {
    case ADD:
      carry = (num + num2) > 0xFF ? 1 : 0;
      break;
    case SUBTRACT:
      carry = (num < num2) ? 1 : 0;
      break;
    case RLC:
      carry = (num & 0x80) >> 7;
      break;
    case AND:
      carry = 0;
      break;
    case XOR:
      carry = 0;
      break;
  }

  return carry;
}

int _8080::check_carry_flag(u16 num, u16 num2, Operation operation) {
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

