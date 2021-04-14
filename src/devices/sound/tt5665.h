// license:BSD-3-Clause
// copyright-holders:Mirko Buffoni,Aaron Giles,cam900
/***************************************************************************

    tt5665.h

    Tontek TT5665 ADCPM sound chip.

***************************************************************************/

#ifndef MAME_SOUND_TT5665_H
#define MAME_SOUND_TT5665_H

#pragma once

#include "sound/okiadpcm.h"
#include "dirom.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> tt5665_device

class tt5665_device : public device_t,
						public device_sound_interface,
						public device_rom_interface<24>
{
public:
	enum ss_state
	{
		SS_LOW = 0,
		SS_HIGH = 1
	};

	// construction/destruction
	tt5665_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, ss_state ss, u8 s0_s1)
		: tt5665_device(mconfig, tag, owner, clock)
	{
		config_ss(ss);
		config_s0_s1(s0_s1);
	}
	tt5665_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// inline configuration helpers
	void config_ss(ss_state ss) { assert(!started()); m_ss_state = ss; }
	void config_s0_s1(u8 s0_s1) { assert(!started()); m_s0_s1_state = s0_s1 & 3; }

	// runtime configuration
	void set_ss(bool ss);
	void set_s0_s1(int s0_s1);

	u8 read();
	void write(u8 command);

protected:
	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override;
	virtual void device_register_save(save_registrar &save) override;
	virtual void device_reset() override;
	virtual void device_post_load() override;
	virtual void device_clock_changed() override;

	// device_sound_interface overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

	// device_rom_interface overrides
	virtual void rom_bank_updated() override;

private:
	// a single voice
	class tt5665_voice
	{
	public:
		tt5665_voice();
		void generate_adpcm(device_rom_interface &rom, s32 *buffer);

		oki_adpcm_state m_adpcm;          // current ADPCM state
		bool            m_playing;
		offs_t          m_base_offset;    // pointer to the base memory location
		u32             m_sample;         // current sample number
		u32             m_count;          // total samples to play
		stream_buffer::sample_t m_volume; // output volume

		void register_save(save_registrar &save)
		{
			save.reg(NAME(m_adpcm))
				.reg(NAME(m_playing))
				.reg(NAME(m_sample))
				.reg(NAME(m_count))
				.reg(NAME(m_volume))
				.reg(NAME(m_base_offset));
		}
	};

	// configuration state
	u8              m_ss_state;
	u8              m_s0_s1_state;

	// internal state
	static constexpr int TT5665_VOICES = 4;

	tt5665_voice    m_voice[TT5665_VOICES * 2]; // separated voice for left and right output
	s32             m_command;
	sound_stream*   m_stream;
	s32             m_daol_output;
	int             m_daol_timing;

	inline int freq_divider() const { return m_ss_state ? 136 : 170; }

	static const stream_buffer::sample_t s_volume_table[16];
};


// device type definition
DECLARE_DEVICE_TYPE(TT5665, tt5665_device)


#endif // MAME_SOUND_TT5665_H
