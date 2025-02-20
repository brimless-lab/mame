// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi, David Haywood
/******************************************************************************

    Gomoku Narabe Renju
    (c)1981 Nihon Bussan Co.,Ltd.

    Driver by Takahiro Nogi <nogi@kt.rim.or.jp> 1999/11/06 -
    Updated to compile again by David Haywood 19th Oct 2002

******************************************************************************/

#include "emu.h"
#include "gomoku.h"


/******************************************************************************

    palette RAM

******************************************************************************/

void gomoku_state::palette(palette_device &palette) const
{
	const uint8_t *color_prom = memregion("proms")->base();

	for (int i = 0; i < palette.entries(); i++)
	{
		int bit0, bit1, bit2;

		// red component
		bit0 = BIT(*color_prom, 0);
		bit1 = BIT(*color_prom, 1);
		bit2 = BIT(*color_prom, 2);
		int const r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		// green component
		bit0 = BIT(*color_prom, 3);
		bit1 = BIT(*color_prom, 4);
		bit2 = BIT(*color_prom, 5);
		int const g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		// blue component
		bit0 = 0;
		bit1 = BIT(*color_prom, 6);
		bit2 = BIT(*color_prom, 7);
		int const b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_pen_color(i, rgb_t(r, g, b));
		color_prom++;
	}
}


/******************************************************************************

    Tilemap callbacks

******************************************************************************/

TILE_GET_INFO_MEMBER(gomoku_state::get_fg_tile_info)
{
	int code = (m_videoram[tile_index]);
	int attr = (m_colorram[tile_index]);
	int color = (attr& 0x0f);
	int flipyx = (attr & 0xc0) >> 6;

	tileinfo.set(0,
			code,
			color,
			TILE_FLIPYX(flipyx));
}

void gomoku_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

void gomoku_state::colorram_w(offs_t offset, uint8_t data)
{
	m_colorram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

void gomoku_state::flipscreen_w(int state)
{
	m_flipscreen = state ? 0 : 1;
}

void gomoku_state::bg_dispsw_w(int state)
{
	m_bg_dispsw = state ? 0 : 1;
}


/******************************************************************************

    Start the video hardware emulation

******************************************************************************/

void gomoku_state::video_start()
{
	m_screen->register_screen_bitmap(m_bg_bitmap);

	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(gomoku_state::get_fg_tile_info)),TILEMAP_SCAN_ROWS,8,8,32, 32);

	m_fg_tilemap->set_transparent_pen(0);

	// make background bitmap
	m_bg_bitmap.fill(0x20);

	// board
	for (int y = 0; y < 256; y++)
	{
		for (int x = 0; x < 256; x++)
		{
			int bgdata = m_bg_d[m_bg_x[x] + (m_bg_y[y] << 4)];

			int color = 0x20;                       // outside frame (black)

			if (bgdata & 0x01) color = 0x21;    // board (brown)
			if (bgdata & 0x02) color = 0x20;    // frame line (while)

			m_bg_bitmap.pix((255 - y - 1) & 0xff, (255 - x + 7) & 0xff) = color;
		}
	}

	save_item(NAME(m_flipscreen)); // set but never used?
	save_item(NAME(m_bg_dispsw));
}


/******************************************************************************

    Display refresh

******************************************************************************/

uint32_t gomoku_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int color;

	// draw background layer
	if (m_bg_dispsw)
	{
		// copy bg bitmap
		copybitmap(bitmap, m_bg_bitmap, 0, 0, 0, 0, cliprect);

		// stone
		for (int y = 0; y < 256; y++)
		{
			for (int x = 0; x < 256; x++)
			{
				int bgoffs = ((((255 - x - 2) / 14) | (((255 - y - 10) / 14) << 4)) & 0xff);

				int bgdata = m_bg_d[m_bg_x[x] + (m_bg_y[y] << 4) ];
				int bgram = m_bgram[bgoffs];

				if (bgdata & 0x04)
				{
					if (bgram & 0x01)
					{
						color = 0x2f;   // stone (black)
					}
					else if (bgram & 0x02)
					{
						color = 0x22;   // stone (white)
					}
					else continue;
				}
				else continue;

				bitmap.pix((255 - y - 1) & 0xff, (255 - x + 7) & 0xff) = color;
			}
		}

		// cursor
		for (int y = 0; y < 256; y++)
		{
			for (int x = 0; x < 256; x++)
			{
				int bgoffs = ((((255 - x - 2) / 14) | (((255 - y - 10) / 14) << 4)) & 0xff);

				int bgdata = m_bg_d[m_bg_x[x] + (m_bg_y[y] << 4) ];
				int bgram = m_bgram[bgoffs];

				if (bgdata & 0x08)
				{
					if (bgram & 0x04)
					{
						color = 0x2f;   // cursor (black)
					}
					else if (bgram & 0x08)
					{
						color = 0x22;       // cursor (white)
					}
					else continue;
				}
				else continue;

				bitmap.pix((255 - y - 1) & 0xff, (255 - x + 7) & 0xff) = color;
			}
		}
	}
	else
	{
		bitmap.fill(0x20);
	}

	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
