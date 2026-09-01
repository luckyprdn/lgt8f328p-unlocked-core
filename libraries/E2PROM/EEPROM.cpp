#include "Arduino.h"
#include "EEPROM.h"

#if defined( __LGT8FX8P__ ) || defined( __LGT_EEPROM_LIB_FOR_328D__ )
static inline void lgt_eeprom_write_eccr(uint8_t value)
{
	uint8_t sreg = SREG;
	cli();
	ECCR = 0x80;
	ECCR = (uint8_t)(value & 0x7Fu);
	SREG = sreg;
}
#endif

#if defined( __LGT8FX8P__ ) || defined( __LGT_EEPROM_LIB_FOR_328D__ )
// EEPM[3:0] is sticky controller state.  Always select the access width
// explicitly instead of inheriting the mode left by a previous byte/word
// operation.  Preserve EERIE so a library call cannot silently disable an
// application-owned EEPROM-ready interrupt.
static inline void lgt_eeprom_issue_read(uint8_t mode)
{
	const uint8_t eerie = (uint8_t)(EECR & 0x08u);
	EECR = (uint8_t)(eerie | (mode & 0xF0u) | 0x01u); // EERE
	__asm__ __volatile__ ("nop" ::);
	__asm__ __volatile__ ("nop" ::);
}

static inline void lgt_eeprom_issue_program(uint8_t mode)
{
	const uint8_t eerie = (uint8_t)(EECR & 0x08u);
	uint8_t sreg = SREG;
	cli();
	EECR = (uint8_t)(eerie | (mode & 0xF0u) | 0x04u); // EEMPE
	EECR = (uint8_t)(eerie | (mode & 0xF0u) | 0x02u); // EEPE
	SREG = sreg;
}
#endif

#if defined( __LGT8FX8P__ )
void lgt_eeprom_init( uint8_t number_of_1KB_pages )
{
#if defined(LGT8_UNLOCKED_RECOVERY_SAFE) && LGT8_UNLOCKED_RECOVERY_SAFE
	// LGT8F328P EEPROM emulation shares the top of program Flash.  In the
	// recovery-safe profile, keep the factory/default 1KB logical partition
	// (2KB physical Flash) so runtime API calls cannot grow the EEPROM into
	// the bootloader or application image.
	(void)number_of_1KB_pages;
	number_of_1KB_pages = 1u;
#endif
	switch( number_of_1KB_pages )
	{
		case 0: // disable EEPROM emulation
			lgt_eeprom_write_eccr(0x00);
		break;

		case 2: // 2KB EEPROM emulation (uses 4KB of data in main Flash memory )
			lgt_eeprom_write_eccr((uint8_t)(0x4C | 0x01));
		break;

		case 4: // 4KB EEPROM emulation (uses 8KB of data in main Flash memory )
			lgt_eeprom_write_eccr((uint8_t)(0x4C | 0x02));
		break;

		case 8: // 8KB EEPROM emulation (uses 16KB of data in main Flash memory )
			lgt_eeprom_write_eccr((uint8_t)(0x4C | 0x03));
		break;
		
		default: // 1KB EEPROM emulation (uses 2KB of data in main Flash memory )
			lgt_eeprom_write_eccr(0x4C);
		break;
	}
}
#elif defined( __LGT_EEPROM_LIB_FOR_328D__ )
void lgt_eeprom_init( uint8_t number_of_1KB_pages )
{
	switch( number_of_1KB_pages )
	{
		case 0: // disable EEPROM emulation
			lgt_eeprom_write_eccr(0x00);
		break;

		case 2: // 2KB EEPROM emulation (uses 4KB of data in main Flash memory )
			lgt_eeprom_write_eccr((uint8_t)(0x40 | 0x01));
		break;

		case 4: // 4KB EEPROM emulation (uses 8KB of data in main Flash memory )
			lgt_eeprom_write_eccr((uint8_t)(0x40 | 0x02));
		break;

		case 8: // 8KB EEPROM emulation (uses 16KB of data in main Flash memory )
			lgt_eeprom_write_eccr((uint8_t)(0x40 | 0x03));
		break;
		
		default: // 1KB EEPROM emulation (uses 2KB of data in main Flash memory )
			lgt_eeprom_write_eccr(0x40);
		break;
	}
}
#endif


int lgt_eeprom_size( bool theoretical )
{
#if defined( __LGT8FX8P__ ) || defined( __LGT_EEPROM_LIB_FOR_328D__ )

	if ( ECCR & 0x40 ) // EEPROM emulation enabled ?
	{
		return theoretical ?
			( 1024 << ( ECCR & 0x3 ) ) // thoerical size of the emulated EEPROM (see notes above)
			:
			( lgt_eeprom_free_space_per_1KB_page() << ( ECCR & 0x3 ) ) // actual number of bytes available to the user (see notes above)
			;
	}

	return 0;

#elif defined( __LGT8F88A__ )
	// The EEPROM is emulated using Flash too. 
	// Some bytes of the EEPROM are used to store fuse and flags.
	// Only 504 bytes are actually available for write.
	
	return theoretical ? 512 : 504; 

#else
	// other undetected boards ?
	
	(void)(theoretical); // suppress unused parameter warning
	
	#ifdef E2END
		return E2END+1;
	#else
		return 1024;
	#endif
#endif
}


#if defined( __LGT8FX8P__ ) || defined( __LGT_EEPROM_LIB_FOR_328D__ )
static bool lgt_eeprom_address_is_user_accessible(uint16_t address, bool real_address_mode)
{
	if (real_address_mode) {
		const uint16_t physical_size = (uint16_t)lgt_eeprom_size(true);
		return address < physical_size && (address % 1024u) < lgt_eeprom_free_space_per_1KB_page();
	}
	return address < (uint16_t)lgt_eeprom_size(false);
}
static bool lgt_eeprom_range_is_user_accessible(uint16_t address, uint16_t len, bool real_address_mode)
{
	if (len == 0) return true;
	if (!lgt_eeprom_address_is_user_accessible(address, real_address_mode)) return false;
	if (real_address_mode) {
		const uint16_t physical_size = (uint16_t)lgt_eeprom_size(true);
		if (len > (uint16_t)(physical_size - address)) return false;
		return len <= (uint16_t)(lgt_eeprom_free_space_per_1KB_page() - (address % 1024u));
	}
	const uint16_t logical_size = (uint16_t)lgt_eeprom_size(false);
	return len <= (uint16_t)(logical_size - address);
}
#endif

#if defined(__LGT8FX8P__) || defined( __LGT_EEPROM_LIB_FOR_328D__ )
uint16_t lgt_eeprom_continuous_address_to_real_address( uint16_t address )
{
	// we recalculate the address so that we automatically skip 
	// every reserved last cell of every 1KB page (see notes above)

	if ( address >= lgt_eeprom_free_space_per_1KB_page() )
	{
		address = ( 1024 * ( address / lgt_eeprom_free_space_per_1KB_page() ) ) // selects the approriate 1KB page 
		        + ( address % lgt_eeprom_free_space_per_1KB_page() ) // selects the approriate byte on this 1KB page
		        ;
	}

	return address;
}
#endif

#if defined( __LGT8FX8P__ ) || defined( __LGT_EEPROM_LIB_FOR_328D__ )
uint8_t lgt_eeprom_read_byte( uint16_t address, bool real_address_mode )
{
	if (!lgt_eeprom_address_is_user_accessible(address, real_address_mode)) return 0;
	if (!real_address_mode) address = lgt_eeprom_continuous_address_to_real_address(address);

	EEARL = address & 0xff;
	EEARH = (address >> 8); 
	 
	lgt_eeprom_issue_read(0x00u); // explicit 8-bit mode
	return EEDR;
}
#else
uint8_t lgt_eeprom_read_byte( uint16_t address )
{
	if ( address >= (uint16_t)lgt_eeprom_size( false ) ) return 0;
	
	EEARL = address & 0xff;
	EEARH = (address >> 8); 
	 
	EECR |= (1 << EERE);
	__asm__ __volatile__ ("nop" ::);
	__asm__ __volatile__ ("nop" ::);
	
	return EEDR;
}
#endif

#if defined( __LGT8FX8P__ ) || defined( __LGT_EEPROM_LIB_FOR_328D__ )
void lgt_eeprom_write_byte( uint16_t address, uint8_t value, bool real_address_mode )
{
	if (!lgt_eeprom_address_is_user_accessible(address, real_address_mode)) return;
	if (!real_address_mode) address = lgt_eeprom_continuous_address_to_real_address(address);

	// Set address/data and explicitly return the controller to 8-bit mode.
	EEARL = (uint8_t)address;
	EEARH = (uint8_t)(address >> 8);
	EEDR = value;
	lgt_eeprom_issue_program(0x00u);
}
#else
void lgt_eeprom_write_byte( uint16_t address, uint8_t value )
{
	if ( address >= (uint16_t)lgt_eeprom_size( false ) ) return;
	
	uint8_t	__bk_sreg = SREG;

	// set address & data
	EEARL = address & 0xff;
	EEARH = (address >> 8);
	EEDR = value;
	 
	cli();
	EECR = 0x04;
	EECR = 0x02;
	SREG = __bk_sreg;
}
#endif

#if defined( __LGT8FX8P__ ) || defined( __LGT_EEPROM_LIB_FOR_328D__ )
void lgt_eeprom_read_block( uint8_t *pbuf, uint16_t address, uint16_t len, bool real_address_mode )
{
	uint16_t i;
	if (pbuf == 0 || !lgt_eeprom_range_is_user_accessible(address, len, real_address_mode)) return;
	uint8_t *p = pbuf;

	for(i = 0; i < len; i++) {
		*p++ = lgt_eeprom_read_byte(address+i, real_address_mode );
	}
}
#else
void lgt_eeprom_read_block( uint8_t *pbuf, uint16_t address, uint16_t len )
{
	uint16_t i;

	uint8_t *p = pbuf;

	for(i = 0; i < len; i++) {
		*p++ = lgt_eeprom_read_byte(address+i);
	}
}
#endif

#if defined( __LGT8FX8P__ ) || defined( __LGT_EEPROM_LIB_FOR_328D__ )
void lgt_eeprom_write_block( uint8_t *pbuf, uint16_t address, uint16_t len, bool real_address_mode )
{
	uint16_t i;
	if (pbuf == 0 || !lgt_eeprom_range_is_user_accessible(address, len, real_address_mode)) return;
	uint8_t *p = pbuf;

	for(i = 0; i < len; i++) {
		lgt_eeprom_write_byte( address+i, *p++, real_address_mode );
	}
}
#else
void lgt_eeprom_write_block( uint8_t *pbuf, uint16_t address, uint16_t len )
{
	uint16_t i;

	uint8_t *p = pbuf;

	for(i = 0; i < len; i++) {
		lgt_eeprom_write_byte( address+i, *p++ );
	}
}
#endif

#if defined(__LGT8FX8P__)
	static bool lgt_eeprom_valid_word_range(uint16_t address, uint16_t words)
	{
		const uint16_t size = (uint16_t)lgt_eeprom_size(false);
		if (words == 0) return true;
		if ((address & 0x03u) != 0) return false;
		if (address >= size) return false;
		const uint32_t bytes = (uint32_t)words * 4UL;
		return bytes <= (uint32_t)(size - address);
	}

	uint32_t lgt_eeprom_read32( uint16_t address )
	{
		uint32_t dwTmp = 0;
		if (!lgt_eeprom_valid_word_range(address, 1)) return 0;
		address = lgt_eeprom_continuous_address_to_real_address(address);

		EEARL = (uint8_t)address;
		EEARH = (uint8_t)(address >> 8);
		lgt_eeprom_issue_read(0x40u); // explicit native 32-bit mode

		dwTmp = E2PD0;
		dwTmp |= ((uint32_t)E2PD1 << 8);
		dwTmp |= ((uint32_t)E2PD2 << 16);
		dwTmp |= ((uint32_t)E2PD3 << 24);
		return dwTmp;
	}

	void lgt_eeprom_write32( uint16_t address, uint32_t value )
	{
		if (!lgt_eeprom_valid_word_range(address, 1)) return;
		address = lgt_eeprom_continuous_address_to_real_address(address);

		// Native 32-bit mode has dedicated data registers.  Do not overload
		// EEARL as an EEDR byte selector here; keeping the data and address
		// paths separate makes the alignment contract explicit.
		E2PD0 = (uint8_t)value;
		E2PD1 = (uint8_t)(value >> 8);
		E2PD2 = (uint8_t)(value >> 16);
		E2PD3 = (uint8_t)(value >> 24);

		EEARH = (uint8_t)(address >> 8);
		EEARL = (uint8_t)address;
		lgt_eeprom_issue_program(0x40u);
	}

	// Write a logical continuous range while respecting the two controller
	// metadata bytes at the end of each physical 1KB emulation page.  SWM is
	// restarted at each page boundary so the hardware never streams through
	// the reserved tail.
	void lgt_eeprom_writeSWM( uint16_t address, uint32_t *pData, uint16_t length )
	{
		if (length == 0 || pData == 0) return;
		if (!lgt_eeprom_valid_word_range(address, length)) return;

		const uint16_t pageBytes = lgt_eeprom_free_space_per_1KB_page();
		uint16_t done = 0;
		while (done < length) {
			uint16_t logical = address + (uint16_t)(done * 4u);
			uint16_t inPage = logical % pageBytes;
			uint16_t wordsHere = (pageBytes - inPage) / 4u;
			uint16_t remaining = length - done;
			if (wordsHere > remaining) wordsHere = remaining;
			uint16_t real = lgt_eeprom_continuous_address_to_real_address(logical);

			lgt_eeprom_reset();
			lgt_eeprom_SWM_ON();
			for (uint16_t i = 0; i < wordsHere; ++i) {
				// The databook documents each SWM programming operation with an
				// explicit EEAR target; it does not promise EEAR auto-increment.
				// Advance the physical address in software instead of depending on
				// undocumented controller state.
				uint16_t wordReal = (uint16_t)(real + (uint16_t)(i * 4u));
				EEARH = (uint8_t)(wordReal >> 8);
				EEARL = (uint8_t)wordReal;

				uint32_t value = pData[done + i];
				E2PD0 = (uint8_t)value;
				E2PD1 = (uint8_t)(value >> 8);
				E2PD2 = (uint8_t)(value >> 16);
				E2PD3 = (uint8_t)(value >> 24);

				if (i == (uint16_t)(wordsHere - 1u)) lgt_eeprom_SWM_OFF();
				lgt_eeprom_issue_program(0x40u);
			}
			done += wordsHere;
		}
	}

	void lgt_eeprom_readSWM( uint16_t address, uint32_t *pData, uint16_t length )
	{
		if (length == 0 || pData == 0) return;
		if (!lgt_eeprom_valid_word_range(address, length)) return;

		// SWM is a *programming* optimisation in the databook; reads do not
		// need it.  Keeping SWM disabled avoids hidden controller state and
		// makes each 32-bit read independently addressable/page-safe.
		for (uint16_t i = 0; i < length; ++i) {
			pData[i] = lgt_eeprom_read32((uint16_t)(address + (uint16_t)(i * 4u)));
		}
	}
#else
	uint32_t lgt_eeprom_read32( uint16_t address )
	{ 
		// Emulation :
		uint32_t data;
		lgt_eeprom_read_block( (uint8_t*)&data, address, sizeof( data ) );
		return data; 
	}
	void     lgt_eeprom_write32( uint16_t address, uint32_t data ) 
	{ 
		// Emulation :
		lgt_eeprom_write_block( (uint8_t*)&data, address, sizeof(data) );
	}

	void lgt_eeprom_writeSWM( uint16_t address, uint32_t *pdata, uint16_t len) 
	{ 
		// Emulation :
		lgt_eeprom_write_block( (uint8_t*)pdata, address, len*sizeof(uint32_t) );
	}
	void  lgt_eeprom_readSWM( uint16_t address, uint32_t *pdata, uint16_t len) 
	{ 
		// Emulation :
		lgt_eeprom_read_block( (uint8_t*)pdata, address, len*sizeof(uint32_t) );
	}
#endif



