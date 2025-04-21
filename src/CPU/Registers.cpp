#include "Registers.hpp"

Registers::Registers(){
    
    window = SDL_CreateWindow("Registers", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_w, window_h, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    int window_x;
    int window_y;

    SDL_GetWindowPosition(window, &window_x, &window_y);
    SDL_SetWindowPosition(window, window_x * 0.70, window_y);

    // font library
    std::string font_file = "../font/Cascadia.ttf";
    font = TTF_OpenFont(font_file.c_str(), 16);
    if (!font) {
        printf("error opening font");
    }
}

bool Registers::check_flag(int flag_distance) {
    u8 mask = (f >> flag_distance);
    if ((mask & 0x1) != 0) {  
        return true;
    } else {
        return false;
    }
}

int Registers::get_flag(int flag_distance) {
    return ((f >> flag_distance) & 0x1);
}

void Registers::set_flag(int flag_distance) {
    if (flag_distance < 0 || flag_distance > 7) {
        return;
    } else {
        u8 mask = (1 << flag_distance);
        f |= mask;
    }
}

void Registers::reset_flag(int flag_distance) {
    if (flag_distance < 0 || flag_distance > 7) {
        return;
    } else {
        u8 mask = ~(1 << flag_distance);
        f &= mask;
    }
}

std::string Registers::get_hex_string(int reg_num) {
    std::stringstream stream;

    switch (reg_num) {
        // 16 bit registers
        case 0:
            stream << "PC: 0x" << std::setfill('0') << std::setw(4) << std::hex << std::uppercase << int(pc);
            return stream.str();
        case 1:
            stream << "SP: 0x" << std::setfill('0') << std::setw(4) << std::hex << std::uppercase << int(sp);
            return stream.str();
        case 2:
            stream << "PSW: 0x" << std::setfill('0') << std::setw(4) << std::hex << std::uppercase << int(PSW);
            return stream.str();
        
        // 8 bit registers
        case 3:
            stream << "A: 0x" << std::setfill('0') << std::setw(2) << std::hex << std::uppercase << int(a);
            return stream.str();
        case 4:
            stream << "F: 0x" << std::setfill('0') << std::setw(2) << std::hex << std::uppercase << int(f);
            return stream.str();
        case 5:
            stream << "B: 0x" << std::setfill('0') << std::setw(2) << std::hex << std::uppercase << int(b);
            return stream.str();
        case 6:
            stream << "C: 0x" << std::setfill('0') << std::setw(2) << std::hex << std::uppercase << int(c);
            return stream.str();
        case 7:
            stream << "D: 0x" << std::setfill('0') << std::setw(2) << std::hex << std::uppercase << int(d);
            return stream.str();
        case 8:
            stream << "E: 0x" << std::setfill('0') << std::setw(2) << std::hex << std::uppercase << int(e);
            return stream.str();
        case 9:
            stream << "L: 0x" << std::setfill('0') << std::setw(2) << std::hex << std::uppercase << int(l);
            return stream.str();
        case 10:
            stream << "H: 0x" << std::setfill('0') << std::setw(2) << std::hex << std::uppercase << int(h);
            return stream.str();
        
        // 1 bit flag
        case 11:
            stream << "carry: " << int(get_flag(CARRY_POS));
            return stream.str();
        case 12:
            stream << "parity: " << int(get_flag(PARITY_POS));
            return stream.str();
        case 13:
            stream << "aux car: " << int(get_flag(AUX_POS));
            return stream.str();
        case 14:
            stream << "zero: " << int(get_flag(ZERO_POS));
            return stream.str();
        case 15:
            stream << "sign: " << int(get_flag(SIGN_POS));
                return stream.str();
        default:
            return "";
    }
}

void Registers::render_regs() {
    int y = 0;
    SDL_Color color = {255, 255, 255};

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    for (int i = 0; i < 16; i++){
        std::string reg = get_hex_string(i);
        SDL_Surface* text = TTF_RenderText_Solid(font, reg.c_str(), color);
        SDL_Texture* texture = SDL_CreateTextureFromSurface( renderer, text );
        SDL_Rect text_rect = {0, y, text->w, text->h};
        SDL_RenderCopy(renderer, texture, NULL, &text_rect);
        y += 20;
        SDL_DestroyTexture( texture );
        SDL_FreeSurface( text ); 
    } 
    SDL_RenderPresent(renderer);
}