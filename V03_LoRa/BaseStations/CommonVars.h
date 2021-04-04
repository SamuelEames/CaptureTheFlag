// Extra variables because apparently Arduino doesn't accept enum types in .ino files


// Game State
enum gameStates {ST_DISABLED, ST_UNLOCKED, ST_BATTLE, ST_OUTCOME, ST_LOCKDOWN, ACT_CARDTAP};
enum NFC_State {READY_FOR_TAG, TAG_READ, TAG_VALID, TAG_RETAPPED, TAG_NEW, WAIT_FOR_LORA, WRITE_TO_CARD};

// enum tribes {AETOS, ARKOUDA, ELAFI, FIDI, KEROS, LYKOS, TARI, TAVROS, NULLTRIBE};


enum CashOperations {OP_add, OP_subtract, OP_multiply, OP_divide, OP_random, OP_set};

