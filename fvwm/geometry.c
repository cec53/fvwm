/* -*-c-*- */
/* This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307	 USA
 */

/* ---------------------------- included header files ----------------------- */

#include "config.h"

#include <stdio.h>

#include "libs/fvwmlib.h"
#include "fvwm.h"
#include "externs.h"
#include "cursor.h"
#include "functions.h"
#include "bindings.h"
#include "misc.h"
#include "screen.h"
#include "geometry.h"
#include "module_interface.h"

/* ---------------------------- local definitions --------------------------- */

/* ---------------------------- local macros -------------------------------- */

/* ---------------------------- imports ------------------------------------- */

/* ---------------------------- included code files ------------------------- */

/* ---------------------------- local types --------------------------------- */

struct _gravity_offset
{
	int x, y;
};

/* ---------------------------- forward declarations ------------------------ */

/* ---------------------------- local variables ----------------------------- */

/* ---------------------------- exported variables (globals) ---------------- */

/* ---------------------------- local functions ----------------------------- */

/* ---------------------------- interface functions ------------------------- */

/************************************************************************
 *
 *  Procedure:
 *	gravity_get_offsets - map gravity to (x,y) offset signs for adding
 *		to x and y when window is mapped to get proper placement.
 *
 ************************************************************************/
void gravity_get_offsets(int grav, int *xp,int *yp)
{
	static struct _gravity_offset gravity_offsets[11] =
		{
			{  0,  0 },	/* ForgetGravity */
			{ -1, -1 },	/* NorthWestGravity */
			{  0, -1 },	/* NorthGravity */
			{  1, -1 },	/* NorthEastGravity */
			{ -1,  0 },	/* WestGravity */
			{  0,  0 },	/* CenterGravity */
			{  1,  0 },	/* EastGravity */
			{ -1,  1 },	/* SouthWestGravity */
			{  0,  1 },	/* SouthGravity */
			{  1,  1 },	/* SouthEastGravity */
			{  0,  0 },	/* StaticGravity */
		};

	if (grav < ForgetGravity || grav > StaticGravity)
	{
		*xp = *yp = 0;
	}
	else
	{
		*xp = (int)gravity_offsets[grav].x;
		*yp = (int)gravity_offsets[grav].y;
	}

	return;
}

/* Move a rectangle while taking gravity into account. */
void gravity_move(int gravity, rectangle *rect, int xdiff, int ydiff)
{
	int xoff;
	int yoff;

	gravity_get_offsets(gravity, &xoff, &yoff);
	rect->x -= xoff * xdiff;
	rect->y -= yoff * ydiff;

	return;
}

/* Resize rectangle while taking gravity into account. */
void gravity_resize(int gravity, rectangle *rect, int wdiff, int hdiff)
{
	int xoff;
	int yoff;

	gravity_get_offsets(gravity, &xoff, &yoff);
	rect->x -= (wdiff * (xoff + 1)) / 2;
	rect->width += wdiff;
	rect->y -= (hdiff * (yoff + 1)) / 2;
	rect->height += hdiff;

	return;
}

/* Removes decorations from the source rectangle and moves it according to the
 * gravity specification. */
void gravity_get_naked_geometry(
	int gravity, FvwmWindow *t, rectangle *dest_g, rectangle *orig_g)
{
	int xoff;
	int yoff;

	gravity_get_offsets(gravity, &xoff, &yoff);
	dest_g->x = orig_g->x + ((xoff + 1) * (orig_g->width - 1)) / 2;
	dest_g->y = orig_g->y + ((yoff + 1) * (orig_g->height - 1)) / 2;
	dest_g->width = orig_g->width - 2 * t->boundary_width;
	dest_g->height =
		orig_g->height - 2 * t->boundary_width - t->title_g.height;

	return;
}

/* Decorate the rectangle.  Resize and shift it according to gravity. */
void gravity_add_decoration(
	int gravity, FvwmWindow *t, rectangle *dest_g, rectangle *orig_g)
{
	*dest_g = *orig_g;
	gravity_resize(
		gravity, dest_g, 2 * t->boundary_width,
		2 * t->boundary_width + t->title_g.height);

	return;
}

void get_relative_geometry(rectangle *rel_g, rectangle *abs_g)
{
	rel_g->x = abs_g->x - Scr.Vx;
	rel_g->y = abs_g->y - Scr.Vy;
	rel_g->width = abs_g->width;
	rel_g->height = abs_g->height;

	return;
}

void gravity_translate_to_northwest_geometry(
	int gravity, FvwmWindow *t, rectangle *dest_g, rectangle *orig_g)
{
	int xoff;
	int yoff;

	gravity_get_offsets(gravity, &xoff, &yoff);
	dest_g->x = orig_g->x -
		((xoff + 1) * (orig_g->width - 1 + 2 * t->old_bw)) / 2;
	dest_g->y = orig_g->y -
		((yoff + 1) * (orig_g->height - 1 + 2 * t->old_bw)) / 2;
	dest_g->width = orig_g->width;
	dest_g->height = orig_g->height;

	return;
}

void gravity_translate_to_northwest_geometry_no_bw(
	int gravity, FvwmWindow *t, rectangle *dest_g, rectangle *orig_g)
{
	int bw = t->old_bw;

	t->old_bw = 0;
	gravity_translate_to_northwest_geometry(gravity, t, dest_g, orig_g);
	t->old_bw = bw;

	return;
}

void get_shaded_geometry(
	FvwmWindow *tmp_win, rectangle *small_g, rectangle *big_g)
{
	size_borders b;
	/* this variable is necessary so the function can be called with
	 * small_g == big_g */
	int big_height = big_g->height;

	get_window_borders(tmp_win, &b);
	switch (SHADED_DIR(tmp_win))
	{
	case DIR_N:
		small_g->width = big_g->width;
		small_g->height = b.total_size.height;
		small_g->x = big_g->x;
		small_g->y = big_g->y;
		break;
	case DIR_S:
		small_g->width = big_g->width;
		small_g->height = b.total_size.height;
		small_g->x = big_g->x;
		small_g->y = big_g->y + big_height - small_g->height;
		break;
#if 0
	case DIR_W:
	case DIR_E:
	case DIR_NW:
	case DIR_NE:
	case DIR_SW:
	case DIR_SE:
		break;
#endif
	default:
		*small_g = *big_g;
		break;
	}

	return;
}

void get_unshaded_geometry(
	FvwmWindow *tmp_win, rectangle *ret_g)
{
	if (IS_SHADED(tmp_win))
	{
		if (IS_MAXIMIZED(tmp_win))
		{
			*ret_g = tmp_win->max_g;
		}
		else
		{
			*ret_g = tmp_win->normal_g;
		}
		get_relative_geometry(ret_g, ret_g);
	}
	else
	{
		*ret_g = tmp_win->frame_g;
	}

	return;
}

/* returns the dimensions of the borders */
void get_window_borders(
	FvwmWindow *tmp_win, size_borders *borders)
{
	borders->top_left.width = tmp_win->boundary_width;
	borders->bottom_right.width = tmp_win->boundary_width;
	borders->top_left.height = tmp_win->boundary_width;
	borders->bottom_right.height = tmp_win->boundary_width;
	if (HAS_BOTTOM_TITLE(tmp_win))
	{
		borders->bottom_right.height += tmp_win->title_g.height;
	}
	else
	{
		borders->top_left.height += tmp_win->title_g.height;
	}
	borders->total_size.width =
		borders->top_left.width + borders->bottom_right.width;
	borders->total_size.height =
		borders->top_left.height + borders->bottom_right.height;

	return;
}

/* This function returns the geometry of the client window.  If the window is
 * shaded, the unshaded geometry is used instead. */
void get_client_geometry(
	FvwmWindow *tmp_win, rectangle *ret_g)
{
	size_borders borders;

	get_unshaded_geometry(tmp_win, ret_g);
	get_window_borders(tmp_win, &borders);
	ret_g->x += borders.top_left.width;
	ret_g->y += borders.top_left.height;
	ret_g->width -= borders.total_size.width;
	ret_g->height -= borders.total_size.height;

	return;
}

/* update the frame_g according to the window's normal_g or max_g and shaded
 * state */
void update_relative_geometry(FvwmWindow *tmp_win)
{
	get_relative_geometry(
		&tmp_win->frame_g,
		(IS_MAXIMIZED(tmp_win)) ? &tmp_win->max_g : &tmp_win->normal_g);
	if (IS_SHADED(tmp_win))
	{
		get_shaded_geometry(
			tmp_win, &tmp_win->frame_g, &tmp_win->frame_g);
	}

	return;
}

/* update the normal_g or max_g according to the window's current position */
void update_absolute_geometry(FvwmWindow *tmp_win)
{
	rectangle *dest_g;

#if 0
	fprintf(stderr,"uag: called for window '%s'\n", tmp_win->name);
#endif

	/* store orig values in absolute coords */
	dest_g = (IS_MAXIMIZED(tmp_win)) ? &tmp_win->max_g : &tmp_win->normal_g;
	dest_g->x = tmp_win->frame_g.x + Scr.Vx;
	dest_g->y = tmp_win->frame_g.y + Scr.Vy;
	dest_g->width = tmp_win->frame_g.width;
	if (!IS_SHADED(tmp_win))
	{
		dest_g->height = tmp_win->frame_g.height;
	}
	else if (SHADED_DIR(tmp_win) == DIR_S)
	{
		dest_g->y += tmp_win->frame_g.height - dest_g->height;
	}
#if 0
	fprintf(stderr,"     frame:  %5d %5d, %4d x %4d\n", tmp_win->frame_g.x,
		tmp_win->frame_g.y, tmp_win->frame_g.width,
		tmp_win->frame_g.height);
	fprintf(stderr,"     normal: %5d %5d, %4d x %4d\n",
		tmp_win->normal_g.x, tmp_win->normal_g.y,
		tmp_win->normal_g.width, tmp_win->normal_g.height);
	if (IS_MAXIMIZED(tmp_win))
	{
		fprintf(stderr,"     max:    %5d %5d, %4d x %4d, %5d %5d\n",
			tmp_win->max_g.x, tmp_win->max_g.y,
			tmp_win->max_g.width, tmp_win->max_g.height,
			tmp_win->max_offset.x, tmp_win->max_offset.y);
	}
#endif

	return;
}

/* make sure a maximized window and it's normal version are never a page or
 * more apart. */
void maximize_adjust_offset(FvwmWindow *tmp_win)
{
	int off_x;
	int off_y;

	if (!IS_MAXIMIZED(tmp_win))
	{
		/* otherwise we might corrupt the normal_g */
		return;
	}
	off_x = tmp_win->normal_g.x - tmp_win->max_g.x - tmp_win->max_offset.x;
	off_y = tmp_win->normal_g.y - tmp_win->max_g.y - tmp_win->max_offset.y;
#if 0
	fprintf(stderr,"mao: xo=%d, yo=%d\n", off_x, off_y);
#endif
	if (off_x >= Scr.MyDisplayWidth)
	{
		tmp_win->normal_g.x -=
			(off_x / Scr.MyDisplayWidth) * Scr.MyDisplayWidth;
#if 0
		fprintf(stderr, "mao: x -= %d\n",
			(off_x / Scr.MyDisplayWidth) * Scr.MyDisplayWidth);
#endif
	}
	else if (off_x <= -Scr.MyDisplayWidth)
	{
		tmp_win->normal_g.x +=
			((-off_x) / Scr.MyDisplayWidth) * Scr.MyDisplayWidth;
#if 0
		fprintf(stderr, "mao: x += %d\n", ((-off_x) / Scr.MyDisplayWidth) * Scr.MyDisplayWidth);
#endif
	}
	if (off_y >= Scr.MyDisplayHeight)
	{
		tmp_win->normal_g.y -=
			(off_y / Scr.MyDisplayHeight) * Scr.MyDisplayHeight;
#if 0
		fprintf(stderr, "mao: y -= %d\n",
			(off_y / Scr.MyDisplayHeight) * Scr.MyDisplayHeight);
#endif
	}
	else if (off_y <= -Scr.MyDisplayHeight)
	{
		tmp_win->normal_g.y +=
			((-off_y) / Scr.MyDisplayHeight) * Scr.MyDisplayHeight;
#if 0
		fprintf(stderr, "mao: y += %d\n",
			((-off_y) / Scr.MyDisplayHeight) * Scr.MyDisplayHeight);
#endif
	}

	return;
}

/***********************************************************************
 *
 *  Procedure:
 *      constrain_size - adjust the given width and height to account for the
 *              constraints imposed by size hints
 *
 *      The general algorithm, especially the aspect ratio stuff, is
 *      borrowed from uwm's CheckConsistency routine.
*
***********************************************************************/
void constrain_size(
	FvwmWindow *tmp_win, unsigned int *widthp, unsigned int *heightp,
	int xmotion, int ymotion, int flags)
{
#define MAKEMULT(a,b) ((b==1) ? (a) : (((int)((a)/(b))) * (b)) )
	int minWidth, minHeight, maxWidth, maxHeight, xinc, yinc, delta;
	int baseWidth, baseHeight;
	int dwidth = *widthp, dheight = *heightp;
	int roundUpX = 0;
	int roundUpY = 0;
	int old_w = 0;
	int old_h = 0;

	if (IS_MAXIMIZED(tmp_win) && (flags & CS_UPDATE_MAX_DEFECT))
	{
		*widthp += tmp_win->max_g_defect.width;
		*heightp += tmp_win->max_g_defect.height;
		old_w = *widthp;
		old_h = *heightp;
	}
	dwidth -= 2 *tmp_win->boundary_width;
	dheight -= (tmp_win->title_g.height + 2 * tmp_win->boundary_width);

	minWidth = tmp_win->hints.min_width;
	minHeight = tmp_win->hints.min_height;

	maxWidth = tmp_win->hints.max_width;
	maxHeight =  tmp_win->hints.max_height;

	if (maxWidth > tmp_win->max_window_width - 2 * tmp_win->boundary_width)
	{
		maxWidth = tmp_win->max_window_width -
			2 * tmp_win->boundary_width;
	}
	if (maxHeight > tmp_win->max_window_height - 2*tmp_win->boundary_width -
	    tmp_win->title_g.height)
	{
		maxHeight =
			tmp_win->max_window_height - 2*tmp_win->boundary_width -
			tmp_win->title_g.height;
	}

	baseWidth = tmp_win->hints.base_width;
	baseHeight = tmp_win->hints.base_height;

	xinc = tmp_win->hints.width_inc;
	yinc = tmp_win->hints.height_inc;

	/*
	 * First, clamp to min and max values
	 */
	if (dwidth < minWidth)
	{
		dwidth = minWidth;
	}
	if (dheight < minHeight)
	{
		dheight = minHeight;
	}
	if (dwidth > maxWidth)
	{
		dwidth = maxWidth;
	}
	if (dheight > maxHeight)
	{
		dheight = maxHeight;
	}

	/*
	 * Second, round to base + N * inc (up or down depending on resize type)
	 * if rounding up store amount
	 */
	if (!(flags & CS_ROUND_UP))
	{
		dwidth = (((dwidth - baseWidth) / xinc) * xinc) + baseWidth;
		dheight = (((dheight - baseHeight) / yinc) * yinc) + baseHeight;
	}
	else
	{
		roundUpX = dwidth;
		roundUpY = dheight;
		dwidth = (((dwidth - baseWidth + xinc - 1) / xinc) * xinc) +
			baseWidth;
		dheight = (((dheight - baseHeight + yinc - 1) / yinc) * yinc) +
			baseHeight;
		roundUpX = dwidth - roundUpX;
		roundUpY = dheight - roundUpY;
	}

	/*
	 * Step 2a: check we didn't move the edge off screen in interactive
	 * moves
	 */
	if ((flags & CS_ROUND_UP) && Event.type == MotionNotify)
	{
		if (xmotion > 0 && Event.xmotion.x_root < roundUpX)
		{
			dwidth -= xinc;
		}
		else if (xmotion < 0 &&
			 Event.xmotion.x_root >= Scr.MyDisplayWidth - roundUpX)
		{
			dwidth -= xinc;
		}
		if (ymotion > 0 && Event.xmotion.y_root < roundUpY)
		{
			dheight -= yinc;
		}
		else if (ymotion < 0 &&
			 Event.xmotion.y_root >= Scr.MyDisplayHeight - roundUpY)
		{
			dheight -= yinc;
		}
	}

	/*
	 * Step 2b: Check that we didn't violate min and max.
	 */
	if (dwidth < minWidth)
	{
		dwidth += xinc;
	}
	if (dheight < minHeight)
	{
		dheight += yinc;
	}
	if (dwidth > maxWidth)
	{
		dwidth -= xinc;
	}
	if (dheight > maxHeight)
	{
		dheight -= yinc;
	}

	/*
	 * Third, adjust for aspect ratio
	 */
#define maxAspectX tmp_win->hints.max_aspect.x
#define maxAspectY tmp_win->hints.max_aspect.y
#define minAspectX tmp_win->hints.min_aspect.x
#define minAspectY tmp_win->hints.min_aspect.y
	/*
	 * The math looks like this:
	 *
	 * minAspectX    dwidth     maxAspectX
	 * ---------- <= ------- <= ----------
	 * minAspectY    dheight    maxAspectY
	 *
	 * If that is multiplied out, then the width and height are
	 * invalid in the following situations:
	 *
	 * minAspectX * dheight > minAspectY * dwidth
	 * maxAspectX * dheight < maxAspectY * dwidth
	 *
	 */

	if (tmp_win->hints.flags & PAspect)
	{

		if (tmp_win->hints.flags & PBaseSize)
		{
			/*
			 * ICCCM 2 demands that aspect ratio should apply
			 * to width - base_width. To prevent funny results,
			 * we reset PBaseSize in GetWindowSizeHints, if
			 * base is not smaller than min.
			*/
			dwidth -= baseWidth;
			maxWidth -= baseWidth;
			minWidth -= baseWidth;
			dheight -= baseHeight;
			maxHeight -= baseHeight;
			minHeight -= baseHeight;
		}

		if ((minAspectX * dheight > minAspectY * dwidth) &&
		    (xmotion == 0))
		{
			/* Change width to match */
			delta = MAKEMULT(
				minAspectX * dheight / minAspectY - dwidth,
				xinc);
			if (dwidth + delta <= maxWidth)
			{
				dwidth += delta;
			}
		}
		if (minAspectX * dheight > minAspectY * dwidth)
		{
			delta = MAKEMULT(
				dheight - dwidth*minAspectY/minAspectX, yinc);
			if (dheight - delta >= minHeight)
			{
				dheight -= delta;
			}
			else
			{
				delta = MAKEMULT(
					minAspectX*dheight / minAspectY -
					dwidth, xinc);
				if (dwidth + delta <= maxWidth)
				{
					dwidth += delta;
				}
			}
		}

		if ((maxAspectX * dheight < maxAspectY * dwidth) &&
		    (ymotion == 0))
		{
			delta = MAKEMULT(
				dwidth * maxAspectY / maxAspectX - dheight,
				yinc);
			if (dheight + delta <= maxHeight)
			{
				dheight += delta;
			}
		}
		if ((maxAspectX * dheight < maxAspectY * dwidth))
		{
			delta = MAKEMULT(dwidth - maxAspectX*dheight/maxAspectY,
					 xinc);
			if (dwidth - delta >= minWidth)
			{
				dwidth -= delta;
			}
			else
			{
				delta = MAKEMULT(
					dwidth * maxAspectY / maxAspectX -
					dheight, yinc);
				if (dheight + delta <= maxHeight)
				{
					dheight += delta;
				}
			}
		}

		if (tmp_win->hints.flags & PBaseSize)
		{
			dwidth += baseWidth;
			dheight += baseHeight;
		}
	}


	/*
	 * Fourth, account for border width and title height
	 */
	*widthp = dwidth + 2*tmp_win->boundary_width;
	*heightp = dheight + tmp_win->title_g.height +
		2 * tmp_win->boundary_width;
	if (IS_MAXIMIZED(tmp_win) && (flags & CS_UPDATE_MAX_DEFECT))
	{
		/* update size defect for maximized window */
		tmp_win->max_g_defect.width = old_w - *widthp;
		tmp_win->max_g_defect.height = old_h - *heightp;
	}

	return;
}

/* This function does roughly the same as constrain_size, but takes into account
 * that the window shifts according to gravity if constrain_size actually
 * changes the width or height. The frame_g of the window is not changed. The
 * target geometry is expected to be in *rect and will be retured through rect.
 */
void gravity_constrain_size(
	int gravity, FvwmWindow *t, rectangle *rect, int flags)
{
	rectangle old_g = t->frame_g;
	rectangle new_g = *rect;
	int new_width = new_g.width;
	int new_height = new_g.height;

	t->frame_g = *rect;
	if (IS_MAXIMIZED(t) && (flags & CS_UPDATE_MAX_DEFECT))
	{
		gravity_resize(
			gravity, &new_g, t->max_g_defect.width,
			t->max_g_defect.height);
		t->max_g_defect.width = 0;
		t->max_g_defect.height = 0;
		new_width = new_g.width;
		new_height = new_g.height;
	}
	constrain_size(
		t, (unsigned int *)&new_width, (unsigned int *)&new_height, 0,
		0, flags);
	if (new_g.width != new_width || new_g.height != new_height)
	{
		gravity_resize(
			gravity, &new_g, new_width - new_g.width,
			new_height - new_g.height);
	}
	t->frame_g = old_g;
	*rect = new_g;

	return;
}

/* returns the icon title geometry if it is visible */
Bool get_visible_icon_title_geometry(
	FvwmWindow *tmp_win, rectangle *ret_g)
{
	if (HAS_NO_ICON_TITLE(tmp_win) || IS_ICON_UNMAPPED(tmp_win) ||
	    !IS_ICONIFIED(tmp_win))
	{
		memset(ret_g, 0, sizeof(*ret_g));
		return False;
	}
	*ret_g = tmp_win->icon_g.title_w_g;

	return True;
}

/* returns the icon title geometry if it the icon title window exists */
Bool get_icon_title_geometry(
	FvwmWindow *tmp_win, rectangle *ret_g)
{
	if (HAS_NO_ICON_TITLE(tmp_win))
	{
		memset(ret_g, 0, sizeof(*ret_g));
		return False;
	}
	*ret_g = tmp_win->icon_g.title_w_g;

	return True;
}

/* returns the icon picture geometry if it is visible */
Bool get_visible_icon_picture_geometry(
	FvwmWindow *tmp_win, rectangle *ret_g)
{
	if (tmp_win->icon_g.picture_w_g.width == 0 ||
	    IS_ICON_UNMAPPED(tmp_win) || !IS_ICONIFIED(tmp_win))
	{
		memset(ret_g, 0, sizeof(*ret_g));
		return False;
	}
	*ret_g = tmp_win->icon_g.picture_w_g;

	return True;
}

/* returns the icon picture geometry if it is exists */
Bool get_icon_picture_geometry(
	FvwmWindow *tmp_win, rectangle *ret_g)
{
	if (tmp_win->icon_g.picture_w_g.width == 0)
	{
		memset(ret_g, 0, sizeof(*ret_g));
		return False;
	}
	*ret_g = tmp_win->icon_g.picture_w_g;

	return True;
}

/* returns the icon geometry (unexpanded title plus pixmap) if it is visible */
Bool get_visible_icon_geometry(
	FvwmWindow *tmp_win, rectangle *ret_g)
{
	if (IS_ICON_UNMAPPED(tmp_win) || !IS_ICONIFIED(tmp_win))
	{
		memset(ret_g, 0, sizeof(*ret_g));
		return False;
	}
	if (tmp_win->icon_g.picture_w_g.width > 0)
	{
		*ret_g = tmp_win->icon_g.picture_w_g;
		if (!HAS_NO_ICON_TITLE(tmp_win))
		{
			ret_g->height += tmp_win->icon_g.title_w_g.height;
		}
	}
	else if (!HAS_NO_ICON_TITLE(tmp_win))
	{
		*ret_g = tmp_win->icon_g.title_w_g;
	}
	else
	{
		memset(ret_g, 0, sizeof(*ret_g));
		return False;
	}

	return True;
}

/* returns the icon geometry (unexpanded title plus pixmap) if it exists */
Bool get_icon_geometry(
	FvwmWindow *tmp_win, rectangle *ret_g)
{
	if (tmp_win->icon_g.picture_w_g.width > 0)
	{
		*ret_g = tmp_win->icon_g.picture_w_g;
		if (!HAS_NO_ICON_TITLE(tmp_win))
		{
			ret_g->height += tmp_win->icon_g.title_w_g.height;
		}
	}
	else if (!HAS_NO_ICON_TITLE(tmp_win))
	{
		*ret_g = tmp_win->icon_g.title_w_g;
	}
	else
	{
		memset(ret_g, 0, sizeof(*ret_g));
		return False;
	}

	return True;
}

/* Returns the visible geometry of a window or icon.  This can be used to test
 * if this region overlaps other windows. */
Bool get_visible_window_or_icon_geometry(
	FvwmWindow *tmp_win, rectangle *ret_g)
{
	if (IS_ICONIFIED(tmp_win))
	{
		return get_visible_icon_geometry(tmp_win, ret_g);
	}
	*ret_g = tmp_win->frame_g;

	return True;
}

void move_icon_to_position(
	FvwmWindow *tmp_win)
{
	if (tmp_win->icon_g.picture_w_g.width > 0)
	{
		XMoveWindow(
			dpy, tmp_win->icon_pixmap_w,
			tmp_win->icon_g.picture_w_g.x,
			tmp_win->icon_g.picture_w_g.y);
	}
	if (!HAS_NO_ICON_TITLE(tmp_win))
	{
		XMoveWindow(
			dpy, tmp_win->icon_title_w,
			tmp_win->icon_g.title_w_g.x,
			tmp_win->icon_g.title_w_g.y);
	}

	return;
}

void broadcast_icon_geometry(
	FvwmWindow *tmp_win, Bool do_force)
{
	rectangle g;
	Bool rc;

	rc = get_visible_icon_geometry(tmp_win, &g);
	if (rc == True && (!IS_ICON_UNMAPPED(tmp_win) || do_force == True))
	{
		BroadcastPacket(
			M_ICON_LOCATION, 7, tmp_win->w, tmp_win->frame,
			(unsigned long)tmp_win,
			g.x, g.y, g.width, g.height);
	}

	return;
}

void modify_icon_position(
	FvwmWindow *tmp_win, int dx, int dy)
{
	if (tmp_win->icon_g.picture_w_g.width > 0)
	{
		tmp_win->icon_g.picture_w_g.x += dx;
		tmp_win->icon_g.picture_w_g.y += dy;
	}
	if (!HAS_NO_ICON_TITLE(tmp_win))
	{
		tmp_win->icon_g.title_w_g.x += dx;
		tmp_win->icon_g.title_w_g.y += dy;
	}

	return;
}

/* set the icon position to the specified value. take care of the actual icon
 * layout */
void set_icon_position(
	FvwmWindow *tmp_win, int x, int y)
{
	if (tmp_win->icon_g.picture_w_g.width > 0)
	{
		tmp_win->icon_g.picture_w_g.x = x;
		tmp_win->icon_g.picture_w_g.y = y;
	}
	else
	{
		tmp_win->icon_g.picture_w_g.x = 0;
		tmp_win->icon_g.picture_w_g.y = 0;
	}
	if (!HAS_NO_ICON_TITLE(tmp_win))
	{
		tmp_win->icon_g.title_w_g.x = x;
		tmp_win->icon_g.title_w_g.y = y;
	}
	else
	{
		tmp_win->icon_g.title_w_g.x = 0;
		tmp_win->icon_g.title_w_g.y = 0;
	}
	if (tmp_win->icon_g.picture_w_g.width > 0 &&
	    !HAS_NO_ICON_TITLE(tmp_win))
	{
		tmp_win->icon_g.title_w_g.x -=
			(tmp_win->icon_g.title_w_g.width -
			 tmp_win->icon_g.picture_w_g.width) / 2;
		tmp_win->icon_g.title_w_g.y +=
			tmp_win->icon_g.picture_w_g.height;
	}

	return;
}

void set_icon_picture_size(
	FvwmWindow *tmp_win, int w, int h)
{
	if (tmp_win->icon_g.picture_w_g.width > 0)
	{
		tmp_win->icon_g.picture_w_g.width = w;
		tmp_win->icon_g.picture_w_g.height = h;
	}
	else
	{
		tmp_win->icon_g.picture_w_g.width = 0;
		tmp_win->icon_g.picture_w_g.height = 0;
	}

	return;
}

void resize_icon_title_height(FvwmWindow *tmp_win, int dh)
{
	if (!HAS_NO_ICON_TITLE(tmp_win))
	{
		tmp_win->icon_g.title_w_g.height += dh;
	}

	return;
}

/* ---------------------------- builtin commands ---------------------------- */

