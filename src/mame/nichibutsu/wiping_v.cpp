// license:BSD-3-Clause
// copyright-holders:Allard van der Bas
/***************************************************************************

  wiping.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "video/resnet.h"
#include "wiping.h"


/***************************************************************************

  Convert the color PROMs into a more useable format.

***************************************************************************/

void wiping_state::wiping_palette(palette_device &palette) const
{
	const uint8_t *color_prom = memregion("proms")->base();
	static constexpr int resistances_rg[3] = { 1000, 470, 220 };
	static constexpr int resistances_b [2] = { 470, 220 };

	// compute the color output resistor weights
	double rweights[3], gweights[3], bweights[2];
	compute_resistor_weights(0, 255, -1.0,
			3, &resistances_rg[0], rweights, 470, 0,
			3, &resistances_rg[0], gweights, 470, 0,
			2, &resistances_b[0],  bweights, 470, 0);

	// create a lookup table for the palette
	for (int i = 0; i < 0x20; i++)
	{
		int bit0, bit1, bit2;

		// red component
		bit0 = BIT(color_prom[i], 0);
		bit1 = BIT(color_prom[i], 1);
		bit2 = BIT(color_prom[i], 2);
		int const r = combine_weights(rweights, bit0, bit1, bit2);

		// green component
		bit0 = BIT(color_prom[i], 3);
		bit1 = BIT(color_prom[i], 4);
		bit2 = BIT(color_prom[i], 5);
		int const g = combine_weights(gweights, bit0, bit1, bit2);

		// blue component
		bit0 = BIT(color_prom[i], 6);
		bit1 = BIT(color_prom[i], 7);
		int const b = combine_weights(bweights, bit0, bit1);

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	// color_prom now points to the beginning of the lookup table
	color_prom += 0x20;

	// chars use colors 0-15
	for (int i = 0; i < 0x100; i++)
	{
		uint8_t const ctabentry = color_prom[i ^ 0x03] & 0x0f;
		palette.set_pen_indirect(i, ctabentry);
	}

	// sprites use colors 16-31
	for (int i = 0x100; i < 0x200; i++)
	{
		uint8_t const ctabentry = (color_prom[i ^ 0x03] & 0x0f) | 0x10;
		palette.set_pen_indirect(i, ctabentry);
	}
}



WRITE_LINE_MEMBER(wiping_state::flipscreen_w)
{
	m_flipscreen = state;
}


uint32_t wiping_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t *spriteram = m_spriteram;
	int offs;

	for (offs = 0x3ff; offs > 0; offs--)
	{
		int mx,my,sx,sy;

		mx = offs % 32;
		my = offs / 32;

		if (my < 2)
		{
			sx = my + 34;
			sy = mx - 2;
		}
		else if (my >= 30)
		{
			sx = my - 30;
			sy = mx - 2;
		}
		else
		{
			sx = mx + 2;
			sy = my - 2;
		}

		if (m_flipscreen)
		{
			sx = 35 - sx;
			sy = 27 - sy;
		}

		m_gfxdecode->gfx(0)->opaque(bitmap,cliprect,
				m_videoram[offs],
				m_colorram[offs] & 0x3f,
				m_flipscreen,m_flipscreen,
				sx*8,sy*8);
	}

	/* Note, we're counting up on purpose ! */
	/* This way the vacuum cleaner is always on top */
	for (offs = 0x0; offs < 128; offs += 2) {
		int sx,sy,color,flipx,flipy,otherbank;

		sx = spriteram[offs+0x100+1] + ((spriteram[offs+0x81] & 0x01) << 8) - 40;
		sy = 224 - spriteram[offs+0x100];
		color = spriteram[offs+1] & 0x3f;

		otherbank = spriteram[offs+0x80] & 0x01;

		flipy = spriteram[offs] & 0x40;
		flipx = spriteram[offs] & 0x80;

		if (m_flipscreen)
		{
			sy = 208 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		m_gfxdecode->gfx(1)->transmask(bitmap,cliprect,
			(spriteram[offs] & 0x3f) + 64 * otherbank,
			color,
			flipx,flipy,
			sx,sy,
			m_palette->transpen_mask(*m_gfxdecode->gfx(1), color, 0x1f));
	}

	/* redraw high priority chars */
	for (offs = 0x3ff; offs > 0; offs--)
	{
		if (m_colorram[offs] & 0x80)
		{
			int mx,my,sx,sy;

			mx = offs % 32;
			my = offs / 32;

			if (my < 2)
			{
				sx = my + 34;
				sy = mx - 2;
			}
			else if (my >= 30)
			{
				sx = my - 30;
				sy = mx - 2;
			}
			else
			{
				sx = mx + 2;
				sy = my - 2;
			}

			if (m_flipscreen)
			{
				sx = 35 - sx;
				sy = 27 - sy;
			}

			m_gfxdecode->gfx(0)->opaque(bitmap,cliprect,
					m_videoram[offs],
					m_colorram[offs] & 0x3f,
					m_flipscreen,m_flipscreen,
					sx*8,sy*8);
			}
	}


#if 0
{
	int i,j;

	for (i = 0;i < 8;i++)
	{
		for (j = 0;j < 8;j++)
		{
			char buf[40];
			sprintf(buf,"%01x",m_soundregs[i*8+j]&0xf);
			ui_draw_text(buf,j*10,i*8);
		}
	}

	for (i = 0;i < 8;i++)
	{
		for (j = 0;j < 8;j++)
		{
			char buf[40];
			sprintf(buf,"%01x",m_soundregs[0x2000+i*8+j]>>4);
			ui_draw_text(buf,j*10,80+i*8);
		}
	}
}
#endif
	return 0;
}
