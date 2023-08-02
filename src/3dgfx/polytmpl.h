#ifdef _MSC_VER
#pragma warning (disable: 4101)
#endif

#if !defined(GOURAUD) && !defined(TEXMAP) && !defined(ZBUF)
#define NOLERP
#endif

void POLYFILL(struct pvertex *varr, int vnum)
{
	int i, line, top, bot;
	struct pvertex *vlast, *v, *vn, *tab;
	int32_t x, y0, y1, dx, dy, slope, fx, fy;
	int start, len;
	g3d_pixel *fbptr, *pptr, color;
#ifdef GOURAUD
	int32_t r, g, b, dr, dg, db, rslope, gslope, bslope;
#ifdef BLEND_ALPHA
	int32_t a, da, aslope;
#endif
#endif	/* GOURAUD */
#ifdef TEXMAP
	int32_t tu, tv, du, dv, uslope, vslope;
	int tx, ty;
	g3d_pixel texel;
#endif
#ifdef ZBUF
	int32_t z, dz, zslope;
	uint16_t *zptr;
#endif

#if !defined(GOURAUD)
	/* for flat shading we already know the color, so pack it once */
	color = G3D_PACK_RGB(varr[0].r, varr[0].g, varr[0].b);
#endif

	vlast = varr + vnum - 1;
	top = pfill_fb.height;
	bot = 0;

	for(i=0; i<vnum; i++) {
		/* scan the edge between the current and next vertex */
		v = varr + i;
		vn = VNEXT(v);

		if(vn->y == v->y) continue;	// XXX ???

		if(vn->y >= v->y) {
			/* inrementing Y: left side */
			tab = left;
		} else {
			/* decrementing Y: right side, flip vertices to trace bottom->up */
			tab = right;
			v = vn;
			vn = varr + i;
		}

		/* calculate edge slope */
		dx = vn->x - v->x;
		dy = vn->y - v->y;
		slope = (dx << 8) / dy;

#ifdef GOURAUD
		r = v->r << COLOR_SHIFT;
		g = v->g << COLOR_SHIFT;
		b = v->b << COLOR_SHIFT;
		dr = (vn->r << COLOR_SHIFT) - r;
		dg = (vn->g << COLOR_SHIFT) - g;
		db = (vn->b << COLOR_SHIFT) - b;
		rslope = (dr << 8) / dy;
		gslope = (dg << 8) / dy;
		bslope = (db << 8) / dy;
#ifdef BLEND_ALPHA
		a = v->a << COLOR_SHIFT;
		da = (vn->a << COLOR_SHIFT) - a;
		aslope = (da << 8) / dy;
#endif	/* BLEND_ALPHA */
#endif	/* GOURAUD */
#ifdef TEXMAP
		tu = v->u;
		tv = v->v;
		du = vn->u - tu;
		dv = vn->v - tv;
		uslope = (du << 8) / dy;
		vslope = (dv << 8) / dy;
#endif	/* TEXMAP */
#ifdef ZBUF
		z = v->z;
		dz = vn->z - z;
		zslope = (dz << 8) / dy;
#endif	/* ZBUF */

		y0 = (v->y + 0x100) & 0xffffff00;	/* start from the next scanline */
		fy = y0 - v->y;						/* fractional part before the next scanline */
		fx = (fy * slope) >> 8;				/* X adjust for the step to the next scanline */
		x = v->x + fx;						/* adjust X */
		y1 = vn->y & 0xffffff00;			/* last scanline of the edge <= vn->y */

		/* also adjust other interpolated attributes */
#ifdef GOURAUD
		r += (fy * rslope) >> 8;
		g += (fy * gslope) >> 8;
		b += (fy * bslope) >> 8;
#ifdef BLEND_ALPHA
		a += (fy * aslope) >> 8;
#endif	/* BLEND_ALPHA */
#endif	/* GOURAUD */
#ifdef TEXMAP
		tu += (fy * uslope) >> 8;
		tv += (fy * vslope) >> 8;
#endif
#ifdef ZBUF
		z += (fy * zslope) >> 8;
#endif

		line = y0 >> 8;
		if(line < top) top = line;
		if((y1 >> 8) > bot) bot = y1 >> 8;

		if(line > 0) tab += line;

		while(line <= (y1 >> 8) && line < pfill_fb.height) {
			if(line >= 0) {
				int val = x < 0 ? 0 : x >> 8;
				tab->x = val < pfill_fb.width ? val : pfill_fb.width - 1;
#ifdef GOURAUD
				tab->r = r;
				tab->g = g;
				tab->b = b;
#ifdef BLEND_ALPHA
				tab->a = a;
#endif	/* BLEND_ALPHA */
#endif	/* GOURAUD */
#ifdef TEXMAP
				tab->u = tu;
				tab->v = tv;
#endif
#ifdef ZBUF
				tab->z = z;
#endif
				tab++;
			}
			x += slope;
#ifdef GOURAUD
			r += rslope;
			g += gslope;
			b += bslope;
#ifdef BLEND_ALPHA
			a += aslope;
#endif	/* BLEND_ALPHA */
#endif	/* GOURAUD */
#ifdef TEXMAP
			tu += uslope;
			tv += vslope;
#endif	/* TEXMAP */
#ifdef ZBUF
			z += zslope;
#endif	/* ZBUF */
			line++;
		}
	}

	if(top < 0) top = 0;
	if(bot >= pfill_fb.height) bot = pfill_fb.height - 1;

	fbptr = pfill_fb.pixels + top * pfill_fb.width;
	for(i=top; i<=bot; i++) {
		start = left[i].x;
		len = right[i].x - start;
		/* XXX we probably need more precision in left/right.x */

#ifndef NOLERP
		dx = len == 0 ? 256 : (len << 8);
#endif

#ifdef GOURAUD
		r = left[i].r;
		g = left[i].g;
		b = left[i].b;
		dr = right[i].r - r;
		dg = right[i].g - g;
		db = right[i].b - b;
		rslope = (dr << 8) / dx;
		gslope = (dg << 8) / dx;
		bslope = (db << 8) / dx;
#ifdef BLEND_ALPHA
		a = left[i].a;
		da = right[i].a - a;
		aslope = (da << 8) / dx;
#endif	/* BLEND_ALPHA */
#endif	/* GOURAUD */
#ifdef TEXMAP
		tu = left[i].u;
		tv = left[i].v;
		du = right[i].u - tu;
		dv = right[i].v - tv;
		uslope = (du << 8) / dx;
		vslope = (dv << 8) / dx;
#endif	/* TEXMAP */
#ifdef ZBUF
		z = left[i].z;
		dz = right[i].z - z;
		zslope = (dz << 8) / dx;
		zptr = pfill_zbuf + i * pfill_fb.width + start;
#endif	/* ZBUF */

		pptr = fbptr + start;
		while(len-- > 0) {
#if defined(GOURAUD) || defined(TEXMAP) || defined(BLEND_ALPHA) || defined(BLEND_ADD)
			int cr, cg, cb;
#endif
#if defined(BLEND_ALPHA) || defined(BLEND_ADD)
			g3d_pixel fbcol;
#endif
#ifdef BLEND_ALPHA
			int alpha, inv_alpha;
#endif
#ifdef ZBUF
			uint16_t cz = z;
			z += zslope;

			if(cz <= *zptr) {
				*zptr++ = cz;
			} else {
				/* ZFAIL: advance all attributes and continue */
#ifdef GOURAUD
				r += rslope;
				g += gslope;
				b += bslope;
#ifdef BLEND_ALPHA
				a += aslope;
#endif
#endif	/* GOURAUD */
#ifdef TEXMAP
				tu += uslope;
				tv += vslope;
#endif	/* TEXMAP */
				/* skip pixel */
				pptr++;
				zptr++;
				continue;
			}
#endif	/* ZBUF */

#ifdef GOURAUD
			/* we upped the color precision to while interpolating the
			 * edges, now drop the extra bits before packing
			 */
			cr = r < 0 ? 0 : (r >> COLOR_SHIFT);
			cg = g < 0 ? 0 : (g >> COLOR_SHIFT);
			cb = b < 0 ? 0 : (b >> COLOR_SHIFT);
			r += rslope;
			g += gslope;
			b += bslope;
#endif	/* GOURAUD */
#ifdef TEXMAP
			tx = (tu >> (16 - pfill_tex.xshift)) & pfill_tex.xmask;
			ty = (tv >> (16 - pfill_tex.yshift)) & pfill_tex.ymask;
			texel = pfill_tex.pixels[(ty << pfill_tex.xshift) + tx];

			tu += uslope;
			tv += vslope;

#ifndef GOURAUD
			/* for flat textured, cr,cg,cb would not be initialized */
			cr = varr[0].r;
			cg = varr[0].g;
			cb = varr[0].b;
#endif	/* !GOURAUD */
			/* This is not correct, should be /255, but it's much faster
			 * to shift by 8 (/256), and won't make a huge difference
			 */
			/*cr = (cr * G3D_UNPACK_R(texel)) >> 8;
			cg = (cg * G3D_UNPACK_G(texel)) >> 8;
			cb = (cb * G3D_UNPACK_B(texel)) >> 8;*/
			cr = G3D_UNPACK_R(texel);
			cg = G3D_UNPACK_G(texel);
			cb = G3D_UNPACK_B(texel);
#endif	/* TEXMAP */

#ifdef BLEND_ALPHA
#ifdef GOURAUD
			alpha = a >> COLOR_SHIFT;
			inv_alpha = 255 - alpha;
			a += aslope;
#else
			alpha = varr[0].a;
#endif
			inv_alpha = 255 - alpha;
			fbcol = *pptr;
			cr = (cr * alpha + G3D_UNPACK_R(fbcol) * inv_alpha) >> 8;
			cg = (cg * alpha + G3D_UNPACK_G(fbcol) * inv_alpha) >> 8;
			cb = (cb * alpha + G3D_UNPACK_B(fbcol) * inv_alpha) >> 8;
#endif	/* BLEND_ALPHA */
#ifdef BLEND_ADD
			fbcol = *pptr;
			cr += G3D_UNPACK_R(fbcol);
			cg += G3D_UNPACK_G(fbcol);
			cb += G3D_UNPACK_B(fbcol);
#endif	/* BLEND_ADD */

#ifdef DEBUG_OVERDRAW
			*pptr++ += DEBUG_OVERDRAW;
#else
#if defined(GOURAUD) || defined(TEXMAP) || defined(BLEND_ALPHA) || defined(BLEND_ADD)
			if(cr >= 255) cr = 255;
			if(cg >= 255) cg = 255;
			if(cb >= 255) cb = 255;
			color = G3D_PACK_RGB(cr, cg, cb);
#endif
			*pptr++ = color;
#endif
		}
		fbptr += pfill_fb.width;
	}
}

#undef NOLERP
