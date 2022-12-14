static char chall3_hash[] = {
    0xca, 0xec, 0x1d, 0x1a, 0xe7, 0x9d, 0x2c, 0x50,
    0x21, 0x5c, 0x17, 0x14, 0x74, 0x17, 0xe8, 0xd8,
    0xa3, 0x6c, 0x17, 0x8b, 0xb7, 0x83, 0xe6, 0xc0,
    0x01, 0x65, 0x06, 0xda, 0x95, 0x55, 0xd3, 0x26,
    0x10, 0x7a, 0x50, 0xac, 0x50, 0x58, 0xad, 0x0b,
    0x44, 0xa9, 0x25, 0xa4, 0x11, 0x58, 0x64, 0x21,
    0x7b, 0x5e, 0xb5, 0x6b, 0x6c, 0xa4, 0xfe, 0x66,
    0xc8, 0x10, 0x29, 0xff, 0x7d, 0x8f, 0x20, 0x1c 
};

void keygen(uint8_t * key) {
    interp_config cfg = interp_default_config();
    interp_set_config(interp0, 1, &cfg);
    interp_config_set_blend(&cfg, true);
    interp_set_config(interp0, 0, &cfg);

    interp0->base[0] = *((uint32_t *)0x61);
    interp0->base[1] = *((uint32_t *)0x61 + 1);

    for(int i = 0; i < 64; i++) {
        interp0->accum[1] = 255 * i / 64;
        uint32_t *div = (uint32_t *)&(interp0->peek[1]);
        uint8_t *b = (uint8_t*)(div);
        key[i] = b[0] + b[1] + b[2] + b[3];
    }
}

void interpolarize(io_rw_32 accum0, io_rw_32 base0, io_rw_32 accum1, io_rw_32 base1) {
    interp_config cfg = interp_default_config();
    interp_set_config(interp0, 0, &cfg);
    interp_set_config(interp0, 1, &cfg);
    
    interp_set_config(interp1, 0, &cfg);
    interp_set_config(interp1, 1, &cfg);
    
    interp0->accum[0] = accum0;
    interp0->base[0] = base0;
    interp0->accum[1] = accum1;
    interp0->base[1] = base1;

    for(int i = 0; i < 63; i++) {
        uint8_t *refa = (uint8_t*)(interp0->peek[0]);
        uint8_t *refb = (uint8_t*)(interp0->pop[1]);
        
        interp1->accum[0] = *refa;
        interp1->accum[1] = *refb;
        
        *refa = interp1->pop[2];
    }
}

void hash(uint8_t *key, const char *input, uint8_t *output) {
    int len = strlen(input);
    memcpy(output, input, 64);
    memset(&output[len], 64-len, 64-len);
    
    interpolarize((io_rw_32)(output-1), 1, (io_rw_32)(key-1), 1);    
    interpolarize((io_rw_32)output, 1, (io_rw_32)(output-1), 1);
    interpolarize((io_rw_32)(output+63), -1, (io_rw_32)(output+64), -1);
}

void chall3_handler(char *input, int len) {
    char key[64];
    char output[64];

    if (input[len-2] == CARRIAGE_RETURN) { 
        input[len-2] = ' ';
        len -= 2;
    } else if (input[len-1] == CARRIAGE_RETURN || input[len-1] == NEWLINE) {
        input[len-1] = ' ';
        len -= 1;
    }

    if (len <= 0 || len >= (64)) {
        uart_printf("Input needs to > 0 and < 64 characters\r\n");
        return;
    }

    keygen(key);
    hash(key, input, output);
       
    if (memcmp(output, chall3_hash, 64) == 0) {
        uart_printf("Winner winner\r\n");
    } else {
        uart_printf("Nope.\r\n");
     return;
    }

    // [..] successfully unlocked
}
