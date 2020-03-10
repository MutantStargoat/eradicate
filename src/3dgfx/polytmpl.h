static uint32_t SCANEDGE(struct pvertex *v0, struct pvertex *v1, struct pvertex *edge)
{
	int i;
	int32_t x, dx, dy, slope;
#ifdef GOURAUD
	int r, g, b, dr, dg, db;
	int32_t rslope, gslope, bslope;
#ifdef BLEND
	int32_t a, da, aslope;
#endif
#endif	/* GOURAUD */
#ifdef TEXMAP
	int32_t u, v, du, dv, uslope, vslope;
#endif
	int32_t start_idx, end_idx;

	if(v0->y > v1->y) {
		struct pvertex *tmp = v0;
		v0 = v1;
		v1 = tmp;
	}

	x = v0->x;
	dy = v1->y - v0->y;
	dx = v1->x - v0->x;
	slope = (dx << 8) / dy;
#ifdef GOURAUD
	r = (v0->r << COLOR_SHIFT);
	g = (v0->g << COLOR_SHIFT);
	b = (v0->b << COLOR_SHIFT);
	dr = (v1->r << COLOR_SHIFT) - r;
	dg = (v1->g << COLOR_SHIFT) - g;
	db = (v1->b << COLOR_SHIFT) - b;
	rslope = (dr << 8) / dy;
	gslope = (dg << 8) / dy;
	bslope = (db << 8) / dy;
#ifdef BLEND
	a = (v0->a << COLOR_SHIFT);
	da = (v1->a << COLOR_SHIFT) - a;
	aslope = (da << 8) / dy;
#endif	/* BLEND */
#endif	/* GOURAUD */
#ifdef TEXMAP
	u = v0->u;
	v = v0->v;
	du = v1->u - v0->u;
	dv = v1->v - v0->v;
	uslope = (du << 8) / dy;
	vslope = (dv << 8) / dy;
#endif

	start_idx = v0->y >> 8;
	end_idx = v1->y >> 8;

	for(i=start_idx; i<end_idx; i++) {
		edge[i].x = x;
		x += slope;
#ifdef GOURAUD
		/* we'll store the color in the edge tables with COLOR_SHIFT extra bits of precision */
		edge[i].r = r;
		edge[i].g = g;
		edge[i].b = b;
		r += rslope;
		g += gslope;
		b += bslope;
#ifdef BLEND
		edge[i].a = a;
		a += aslope;
#endif
#endif	/* GOURAUD */
#ifdef TEXMAP
		edge[i].u = u;
		edge[i].v = v;
		u += uslope;
		v += vslope;
#endif
	}

	return (uint32_t)start_idx | ((uint32_t)(end_idx - 1) << 16);
}

void POLYFILL(struct pvertex *pv, int nverts)
{
	int i, winding;
	int topidx = 0, botidx = 0, sltop = pfill_fb.height, slbot = 0;
	struct pvertex *left, *right;
	g3d_pixel color;
	/* the following variables are used for interpolating horizontally accros scanlines */
#if defined(GOURAUD) || defined(TEXMAP)
	int mid;
	int32_t dx, tmp;
#else
	/* flat version, just pack the color now */
	color = G3D_PACK_RGB(pv[0].r, pv[0].g, pv[0].b);
#endif
#ifdef GOURAUD
	int32_t r, g, b, dr, dg, db, rslope, gslope, bslope;
#ifdef BLEND
	int32_t a, da, aslope;
#endif
#endif
#ifdef TEXMAP
	int32_t u, v, du, dv, uslope, vslope;
#endif

	for(i=1; i<nverts; i++) {
		if(pv[i].y < pv[topidx].y) topidx = i;
		if(pv[i].y > pv[botidx].y) botidx = i;
	}

	winding = 0;
	for(i=0; i<nverts; i++) {
		int next = NEXTIDX(i);
		winding += ((pv[next].x - pv[i].x) >> 4) * ((pv[next].y + pv[i].y) >> 4);
	}

	/* +padding to avoid crashing due to off-by-one rounding errors in the rasterization */
	left = alloca((pfill_fb.height + 8) * sizeof *left);
	right = alloca((pfill_fb.height + 8) * sizeof *right);
	left += 4;
	right += 4;

	for(i=0; i<nverts; i++) {
		int next = NEXTIDX(i);
		int32_t y0 = pv[i].y;
		int32_t y1 = pv[next].y;

		if((y0 >> 8) == (y1 >> 8)) {
			/*if(y0 > y1) {*/
				int i0, i1;
				int idx = y0 >> 8;
				if(pv[i].x < pv[next].x) {
					i0 = i;
					i1 = next;
				} else {
					i0 = next;
					i1 = i;
				}
				left[idx].x = pv[i0].x;
				right[idx].x = pv[i1].x;
#ifdef GOURAUD
				left[idx].r = pv[i0].r << COLOR_SHIFT;
				left[idx].g = pv[i0].g << COLOR_SHIFT;
				left[idx].b = pv[i0].b << COLOR_SHIFT;
				right[idx].r = pv[i1].r << COLOR_SHIFT;
				right[idx].g = pv[i1].g << COLOR_SHIFT;
				right[idx].b = pv[i1].b << COLOR_SHIFT;
#ifdef BLEND
				left[idx].a = pv[i0].a << COLOR_SHIFT;
				right[idx].a = pv[i1].a << COLOR_SHIFT;
#endif	/* BLEND */
#endif
#ifdef TEXMAP
				left[idx].u = pv[i0].u;
				left[idx].v = pv[i0].v;
				right[idx].u = pv[i1].u;
				right[idx].v = pv[i1].v;
#endif
				if(idx > slbot) slbot = idx;
				if(idx < sltop) sltop = idx;
			/*}*/
		} else {
			struct pvertex *edge;
			uint32_t res, tmp;

			if(winding < 0) {
				/* clockwise */
				edge = y0 > y1 ? left : right;
			} else {
				/* counter-clockwise */
				edge = y0 > y1 ? right : left;
			}
			res = SCANEDGE(pv + i, pv + next, edge);
			tmp = (res >> 16) & 0xffff;
			if(tmp > slbot) slbot = tmp;
			if((tmp = res & 0xffff) < sltop) {
				sltop = tmp;
			}
		}
	}

	/* calculate the slopes of all attributes across the largest span out
	 * of the three: middle, top, or bottom.
	 */
#ifndef HIGH_QUALITY
#if defined(GOURAUD) || defined(TEXMAP)
	mid = (sltop + slbot) >> 1;
	dx = right[mid].x - left[mid].x;
	if((tmp = right[sltop].x - left[sltop].x) > dx) {
		dx = tmp;
		mid = sltop;
	}
	if((tmp = right[slbot].x - left[slbot].x) > dx) {
		dx = tmp;
		mid = slbot;
	}
	if(!dx) dx = 256;	/* avoid division by zero */
#endif
#ifdef GOURAUD
	dr = right[mid].r - left[mid].r;
	dg = right[mid].g - left[mid].g;
	db = right[mid].b - left[mid].b;
	rslope = (dr << 8) / dx;
	gslope = (dg << 8) / dx;
	bslope = (db << 8) / dx;
#ifdef BLEND
	da = right[mid].a - left[mid].a;
	aslope = (da << 8) / dx;
#endif	/* BLEND */
#endif
#ifdef TEXMAP
	du = right[mid].u - left[mid].u;
	dv = right[mid].v - left[mid].v;
	uslope = (du << 8) / dx;
	vslope = (dv << 8) / dx;
#endif
#endif	/* !defined(HIGH_QUALITY) */

	/* for each scanline ... */
	for(i=sltop; i<=slbot; i++) {
		g3d_pixel *pixptr;
		int32_t x;

		x = left[i].x;
		pixptr = pfill_fb.pixels + i * pfill_fb.width + (x >> 8);

#ifdef GOURAUD
		r = left[i].r;
		g = left[i].g;
		b = left[i].b;
#ifdef BLEND
		a = left[i].a;
#endif	/* BLEND */
#endif
#ifdef TEXMAP
		u = left[i].u;
		v = left[i].v;
#endif

#if defined(HIGH_QUALITY) && (defined(GOURAUD) || defined(TEXMAP))
		if(!(dx = right[i].x - left[i].x)) dx = 256;

#ifdef GOURAUD
		dr = right[i].r - left[i].r;
		dg = right[i].g - left[i].g;
		db = right[i].b - left[i].b;
		rslope = (dr << 8) / dx;
		gslope = (dg << 8) / dx;
		bslope = (db << 8) / dx;
#ifdef BLEND
		da = right[i].a - left[i].a;
		aslope = (da << 8) / dx;
#endif	/* BLEND */
#endif	/* GOURAUD */
#ifdef TEXMAP
		du = right[i].u - left[i].u;
		dv = right[i].v - left[i].v;
		uslope = (du << 8) / dx;
		vslope = (dv << 8) / dx;
#endif
#endif	/* HIGH_QUALITY */

		/* go across the scanline interpolating if necessary */
		while(x <= right[i].x) {
#if defined(GOURAUD) || defined(TEXMAP) || defined(BLEND)
			int cr, cg, cb;
#endif
#ifdef BLEND
			g3d_pixel fbcol;
			int alpha, inv_alpha;
#endif
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
#ifdef BLEND
			a += aslope;
#else
			if(cr > 255) cr = 255;
			if(cg > 255) cg = 255;
			if(cb > 255) cb = 255;
#endif	/* BLEND */
#endif	/* GOURAUD */
#ifdef TEXMAP
			{
				int tx = (u >> (16 - pfill_tex.xshift)) & pfill_tex.xmask;
				int ty = (v >> (16 - pfill_tex.yshift)) & pfill_tex.ymask;
				g3d_pixel texel = pfill_tex.pixels[(ty << pfill_tex.xshift) + tx];
#ifdef GOURAUD
				/* This is not correct, should be /255, but it's much faster
				 * to shift by 8 (/256), and won't make a huge difference
				 */
				cr = (cr * G3D_UNPACK_R(texel)) >> 8;
				cg = (cg * G3D_UNPACK_G(texel)) >> 8;
				cb = (cb * G3D_UNPACK_B(texel)) >> 8;
#else
				cr = G3D_UNPACK_R(texel);
				cg = G3D_UNPACK_G(texel);
				cb = G3D_UNPACK_B(texel);
#endif
			}
			u += uslope;
			v += vslope;
#endif

#ifdef BLEND
#if !defined(GOURAUD) && !defined(TEXMAP)
			/* flat version: cr,cg,cb are uninitialized so far */
			cr = pv[0].r;
			cg = pv[0].g;
			cb = pv[0].b;
#endif
#ifdef GOURAUD
			alpha = a >> COLOR_SHIFT;
#else
			alpha = pv[0].a;
#endif
			fbcol = *pixptr;
			inv_alpha = 255 - alpha;
			cr = (cr * alpha + G3D_UNPACK_R(fbcol) * inv_alpha) >> 8;
			cg = (cg * alpha + G3D_UNPACK_G(fbcol) * inv_alpha) >> 8;
			cb = (cb * alpha + G3D_UNPACK_B(fbcol) * inv_alpha) >> 8;
			if(cr > 255) cr = 255;
			if(cg > 255) cg = 255;
			if(cb > 255) cb = 255;
#endif	/* BLEND */

#if defined(GOURAUD) || defined(TEXMAP) || defined(BLEND)
			color = G3D_PACK_RGB(cr, cg, cb);
#endif

#ifdef DEBUG_OVERDRAW
			*pixptr++ += DEBUG_OVERDRAW;
#else
			*pixptr++ = color;
#endif
			x += 256;
		}
	}
}

