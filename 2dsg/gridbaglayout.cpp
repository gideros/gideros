#include "gridbaglayout.h"
#include "sprite.h"
#include <climits>

#define EMPIRICMULTIPLIER 	2
#define MAXGRIDSIZE 		512
#define MINSIZE				1
#define PREFERREDSIZE		2

GridBagConstraints *GridBagLayout::lookupConstraints(Sprite *comp) {
	if (comp->layoutConstraints)
		return comp->layoutConstraints;
	return &defaultConstraints;
}

void GridBagLayout::preInitMaximumArraySizes(Sprite *parent, size_t &a0,
		size_t &a1) {
	GridBagConstraints *constraints;
    size_t curX, curY;
    size_t curWidth, curHeight;
    size_t preMaximumArrayXIndex = 0;
    size_t preMaximumArrayYIndex = 0;

	for (int compId = 0; compId < parent->childCount(); compId++) {
		Sprite *comp = parent->child(compId);
		if ((!comp->visible())||(!comp->layoutConstraints)) {
			continue;
		}

		constraints = lookupConstraints(comp);
		curX = constraints->gridx;
		curY = constraints->gridy;
		curWidth = constraints->gridwidth;
		curHeight = constraints->gridheight;
        if (curWidth==0) curWidth=1;
        if (curHeight==0) curHeight=1;

		if ((curY + curHeight) > preMaximumArrayXIndex)
			preMaximumArrayXIndex = curY + curHeight;
		if ((curX + curWidth) > preMaximumArrayYIndex)
			preMaximumArrayYIndex = curX + curWidth;
	}
	a0 = preMaximumArrayXIndex;
	a1 = preMaximumArrayYIndex;
} //PreInitMaximumSizes

GridBagLayoutInfo GridBagLayout::getLayoutInfo(Sprite *parent, int sizeflag) {
	Sprite *comp;
	GridBagConstraints *constraints;

	// Code below will address index curX+curWidth in the case of yMaxArray, weightY
	// ( respectively curY+curHeight for xMaxArray, weightX ) where
	//  curX in 0 to preInitMaximumArraySizes.y
	// Thus, the maximum index that could
	// be calculated in the following code is curX+curX.
	// EmpericMultier equals 2 because of this.

    size_t layoutWidth, layoutHeight;
    size_t compindex, i, k, nextSize;
    size_t px,py;
    float pixels_diff;
    size_t curX = 0; // constraints.gridx
    size_t curY = 0; // constraints.gridy
    size_t curWidth = 1;  // constraints.gridwidth
    size_t curHeight = 1;  // constraints.gridheight
	double weight_diff, weight;
    size_t maximumArrayXIndex = 0;
    size_t maximumArrayYIndex = 0;
	float dw = 0, dh = 0;

	/*
	 * Pass #1
	 *
	 * Figure out the dimensions of the layout grid (use a value of 1 for
	 * zero or negative widths and heights).
	 */

	layoutWidth = layoutHeight = 0;
	size_t arraySizes0, arraySizes1;
	preInitMaximumArraySizes(parent, arraySizes0, arraySizes1);

	maximumArrayXIndex =
			(EMPIRICMULTIPLIER * arraySizes0 > INT_MAX) ?
                    INT_MAX : EMPIRICMULTIPLIER * arraySizes0;
	maximumArrayYIndex =
			(EMPIRICMULTIPLIER * arraySizes1 > INT_MAX) ?
                    INT_MAX : EMPIRICMULTIPLIER * arraySizes1;

    if (maximumArrayXIndex < rowHeights.size())
        maximumArrayXIndex = rowHeights.size();
    if (maximumArrayYIndex < columnWidths.size())
        maximumArrayYIndex = columnWidths.size();
    if (maximumArrayXIndex < rowWeights.size())
        maximumArrayXIndex = rowWeights.size();
    if (maximumArrayYIndex < columnWeights.size())
        maximumArrayYIndex = columnWeights.size();

    std::vector<size_t> xMaxArray(maximumArrayXIndex);
    std::vector<size_t> yMaxArray(maximumArrayYIndex);

	for (compindex = 0; compindex < parent->childCount(); compindex++) {
		comp = parent->child(compindex);
		if ((!comp->visible())||(!comp->layoutConstraints))
			continue;
		constraints = lookupConstraints(comp);

		curX = constraints->gridx;
		curY = constraints->gridy;
		curWidth = constraints->gridwidth;
		if (curWidth <= 0)
			curWidth = 1;
		curHeight = constraints->gridheight;
		if (curHeight <= 0)
			curHeight = 1;

		/* Adjust the grid width and height
		 *  fix for 5005945: unneccessary loops removed
		 */
		px = curX + curWidth;
		if (layoutWidth < px) {
			layoutWidth = px;
		}
		py = curY + curHeight;
		if (layoutHeight < py) {
			layoutHeight = py;
		}

		/* Adjust xMaxArray and yMaxArray */
		for (i = curX; i < (curX + curWidth); i++) {
			yMaxArray[i] = py;
		}
		for (i = curY; i < (curY + curHeight); i++) {
			xMaxArray[i] = px;
		}

		/* Cache the current slave's size. */
		if (comp->layoutConstraints) {
			dw = (sizeflag == PREFERREDSIZE) ?
				comp->layoutConstraints->prefWidth :
				comp->layoutConstraints->aminWidth;
			dh = (sizeflag == PREFERREDSIZE) ?
				comp->layoutConstraints->prefHeight :
				comp->layoutConstraints->aminHeight;
			if (sizeflag==PREFERREDSIZE) {
				// If preferred size is unspecified, use minimum size spec first as a fallback
				if (dw==-1) dw=comp->layoutConstraints->aminWidth;
				if (dh==-1) dh=comp->layoutConstraints->aminHeight;
			}
			if ((dw==-1)||(dh==-1)) {
				float diw,dih;
				if (comp->layoutState) {
					GridBagLayoutInfo info = comp->layoutState->getLayoutInfo(comp,
							sizeflag);
					comp->layoutState->getMinSize(comp, info, diw, dih,comp->layoutState->pInsets);
				}
				else
					comp->getMinimumSize(diw,dih,sizeflag==PREFERREDSIZE);
				if (dw==-1) dw=diw;
				if (dh==-1) dh=dih;
			}
		}
		else {
			if (comp->layoutState) {
				GridBagLayoutInfo info = comp->layoutState->getLayoutInfo(comp,
						sizeflag);
				comp->layoutState->getMinSize(comp, info, dw, dh,comp->layoutState->pInsets);
			}
			else
				comp->getMinimumSize(dw,dh,sizeflag==PREFERREDSIZE);
		}
		constraints->minWidth = dw;
		constraints->minHeight = dh;
	} //for (components) loop

	/*
	 * Apply minimum row/column dimensions
	 */
    if (layoutWidth < columnWidths.size())
        layoutWidth = columnWidths.size();
    if (layoutHeight < rowHeights.size())
        layoutHeight = rowHeights.size();
    if (layoutWidth < columnWeights.size())
        layoutWidth = columnWeights.size();
    if (layoutHeight < rowWeights.size())
        layoutHeight = rowWeights.size();

	GridBagLayoutInfo r = GridBagLayoutInfo(layoutWidth, layoutHeight);

	/*
	 * Pass #2
	 *
	 * Negative values for gridX are filled in with the current x value.
	 * Negative values for gridY are filled in with the current y value.
	 * Negative or zero values for gridWidth and gridHeight end the current
	 *  row or column, respectively.
	 */

    for (std::vector<size_t>::iterator it = xMaxArray.begin(), end =
			xMaxArray.end(); it != end; ++it)
		*it = 0;
    for (std::vector<size_t>::iterator it = yMaxArray.begin(), end =
			yMaxArray.end(); it != end; ++it)
		*it = 0;

    for (compindex = 0; compindex < (size_t)parent->childCount(); compindex++) {
        comp = parent->child((int)compindex);
        if ((!comp->visible())||(!comp->layoutConstraints))
            continue;
		constraints = lookupConstraints(comp);

		curX = constraints->gridx;
		curY = constraints->gridy;
		curWidth = constraints->gridwidth;
		curHeight = constraints->gridheight;

		if (curWidth <= 0) {
			curWidth += r.width - curX;
			if (curWidth < 1)
				curWidth = 1;
		}

		if (curHeight <= 0) {
			curHeight += r.height - curY;
			if (curHeight < 1)
				curHeight = 1;
		}

		px = curX + curWidth;
		py = curY + curHeight;

		for (i = curX; i < (curX + curWidth); i++) {
			yMaxArray[i] = py;
		}
		for (i = curY; i < (curY + curHeight); i++) {
			xMaxArray[i] = px;
		}

		/* Assign the new values to the gridbag slave */
		constraints->tempX = curX;
		constraints->tempY = curY;
		constraints->tempWidth = curWidth;
		constraints->tempHeight = curHeight;
	}

	r.weightX.clear();
	r.weightX.resize(maximumArrayYIndex);
	r.weightY.clear();
	r.weightY.resize(maximumArrayXIndex);
	r.minWidth.clear();
	r.minWidth.resize(maximumArrayYIndex);
	r.minHeight.clear();
	r.minHeight.resize(maximumArrayXIndex);

	/*
	 * Apply minimum row/column dimensions and weights
	 */
	for (size_t i = 0; i < columnWidths.size(); i++)
		r.minWidth[i] = columnWidths[i];
	for (size_t i = 0; i < rowHeights.size(); i++)
		r.minHeight[i] = rowHeights[i];
	size_t max =
			columnWeights.size() < r.weightX.size() ?
					columnWeights.size() : r.weightX.size();
	for (size_t i = 0; i < max; i++)
		r.weightX[i] = columnWeights[i];
	max = rowWeights.size() < r.weightY.size() ?
			rowWeights.size() : r.weightY.size();
	for (size_t i = 0; i < max; i++)
		r.weightY[i] = rowWeights[i];

	/*
	 * Pass #3
	 *
	 * Distribute the minimun widths and weights:
	 */

	nextSize = INT_MAX;

	for (i = 1; i != INT_MAX; i = nextSize, nextSize = INT_MAX) {
        for (compindex = 0; compindex < (size_t)parent->childCount(); compindex++) {
            comp = parent->child((int)compindex);
            if ((!comp->visible())||(!comp->layoutConstraints))
                continue;
			constraints = lookupConstraints(comp);

			if (constraints->tempWidth == i) {
				px = constraints->tempX + constraints->tempWidth; /* right column */

				/*
				 * Figure out if we should use this slave\'s weight.  If the weight
				 * is less than the total weight spanned by the width of the cell,
				 * then discard the weight.  Otherwise split the difference
				 * according to the existing weights.
				 */

				weight_diff = constraints->weightx;
				for (k = constraints->tempX; k < px; k++)
					weight_diff -= r.weightX[k];
				if (weight_diff > 0.0) {
					weight = 0.0;
					for (k = constraints->tempX; k < px; k++)
						weight += r.weightX[k];
					for (k = constraints->tempX; weight > 0.0 && k < px; k++) {
						double wt = r.weightX[k];
						double dx = (wt * weight_diff) / weight;
						r.weightX[k] += dx;
						weight_diff -= dx;
						weight -= wt;
					}
					/* Assign the remainder to the rightmost cell */
					r.weightX[px - 1] += weight_diff;
				}

				/*
				 * Calculate the minWidth array values.
				 * First, figure out how wide the current slave needs to be.
				 * Then, see if it will fit within the current minWidth values.
				 * If it will not fit, add the difference according to the
				 * weightX array.
				 */

				pixels_diff = constraints->minWidth + constraints->ipadx
						+ constraints->insets.left + constraints->insets.right;

				for (k = constraints->tempX; k < px; k++)
					pixels_diff -= r.minWidth[k];
				if (pixels_diff > 0) {
					weight = 0.0;
					for (k = constraints->tempX; k < px; k++)
						weight += r.weightX[k];
					for (k = constraints->tempX; weight > 0.0 && k < px; k++) {
						double wt = r.weightX[k];
						double dx = ((wt * ((double) pixels_diff)) / weight);
						r.minWidth[k] += dx;
						pixels_diff -= dx;
						weight -= wt;
					}
					/* Any leftovers go into the rightmost cell */
					r.minWidth[px - 1] += pixels_diff;
				}
			} else if (constraints->tempWidth > i
					&& constraints->tempWidth < nextSize)
				nextSize = constraints->tempWidth;

			if (constraints->tempHeight == i) {
				py = constraints->tempY + constraints->tempHeight; /* bottom row */

				/*
				 * Figure out if we should use this slave's weight.  If the weight
				 * is less than the total weight spanned by the height of the cell,
				 * then discard the weight.  Otherwise split it the difference
				 * according to the existing weights.
				 */

				weight_diff = constraints->weighty;
				for (k = constraints->tempY; k < py; k++)
					weight_diff -= r.weightY[k];
				if (weight_diff > 0.0) {
					weight = 0.0;
					for (k = constraints->tempY; k < py; k++)
						weight += r.weightY[k];
					for (k = constraints->tempY; weight > 0.0 && k < py; k++) {
						double wt = r.weightY[k];
						double dy = (wt * weight_diff) / weight;
						r.weightY[k] += dy;
						weight_diff -= dy;
						weight -= wt;
					}
					/* Assign the remainder to the bottom cell */
					r.weightY[py - 1] += weight_diff;
				}

				/*
				 * Calculate the minHeight array values.
				 * First, figure out how tall the current slave needs to be.
				 * Then, see if it will fit within the current minHeight values.
				 * If it will not fit, add the difference according to the
				 * weightY array.
				 */

				pixels_diff = constraints->minHeight + constraints->ipady
						+ constraints->insets.top + constraints->insets.bottom;
				for (k = constraints->tempY; k < py; k++)
					pixels_diff -= r.minHeight[k];
				if (pixels_diff > 0) {
					weight = 0.0;
					for (k = constraints->tempY; k < py; k++)
						weight += r.weightY[k];
					for (k = constraints->tempY; weight > 0.0 && k < py; k++) {
						double wt = r.weightY[k];
						double dy = ((wt * ((double) pixels_diff)) / weight);
						r.minHeight[k] += dy;
						pixels_diff -= dy;
						weight -= wt;
					}
					/* Any leftovers go into the bottom cell */
					r.minHeight[py - 1] += pixels_diff;
				}
			} else if (constraints->tempHeight > i
					&& constraints->tempHeight < nextSize)
				nextSize = constraints->tempHeight;
		}
	}
	
    //PASS 4: equalize cells if needed
    if (equalizeCells&&(sizeflag==PREFERREDSIZE)) {
        double mw=0;
        for (size_t i = 0; i < maximumArrayYIndex; i++) {
            if (r.weightX[i]>0) {
                double mwt=r.minWidth[i]/r.weightX[i];
                if (mwt>mw) mw=mwt;
            }
        }
        if (mw>0) {
            for (size_t i = 0; i < maximumArrayYIndex; i++) {
                if (r.weightX[i]>0) {
                    r.minWidth[i]=r.weightX[i]*mw;
                }
            }
        }
        mw=0;
        for (size_t i = 0; i < maximumArrayXIndex; i++) {
            if (r.weightY[i]>0) {
                double mwt=r.minHeight[i]/r.weightY[i];
                if (mwt>mw) mw=mwt;
            }
        }
        if (mw>0) {
            for (size_t i = 0; i < maximumArrayXIndex; i++) {
                if (r.weightY[i]>0) {
                    r.minHeight[i]=r.weightY[i]*mw;
                }
            }
        }
    }
	return r;
} //getLayoutInfo()

/**
 * Adjusts the x, y, width, and height fields to the correct
 * values depending on the constraint geometry and pads.
 * This method should only be used internally by
 * <code>GridBagLayout</code>.
 *
 * @param constraints the constraints to be applied
 * @param r the <code>Rectangle</code> to be adjusted
 * @since 1.4
 */
void GridBagLayout::AdjustForGravity(GridBagConstraints *constraints,
		Rectangle &r) {
    float diffx, diffy;

    r.x += constraints->insets.left;
	r.width -= (constraints->insets.left + constraints->insets.right);
	r.y += constraints->insets.top;
	r.height -= (constraints->insets.top + constraints->insets.bottom);

	diffx = 0;
	if ((constraints->fill != GridBagConstraints::HORIZONTAL
			&& constraints->fill != GridBagConstraints::BOTH)
			&& (r.width > (constraints->minWidth + constraints->ipadx))) {
		diffx = r.width - (constraints->minWidth + constraints->ipadx);
		r.width = constraints->minWidth + constraints->ipadx;
	}

	diffy = 0;
	if ((constraints->fill != GridBagConstraints::VERTICAL
			&& constraints->fill != GridBagConstraints::BOTH)
			&& (r.height > (constraints->minHeight + constraints->ipady))) {
		diffy = r.height - (constraints->minHeight + constraints->ipady);
		r.height = constraints->minHeight + constraints->ipady;
	}

	switch (constraints->anchor) {
	case GridBagConstraints::CENTER:
        r.x += diffx*constraints->anchorX;
        r.y += diffy*constraints->anchorY;
		break;
	case GridBagConstraints::NORTH:
		r.x += diffx / 2;
		break;
	case GridBagConstraints::NORTHEAST:
		r.x += diffx;
		break;
	case GridBagConstraints::EAST:
		r.x += diffx;
		r.y += diffy / 2;
		break;
	case GridBagConstraints::SOUTHEAST:
		r.x += diffx;
		r.y += diffy;
		break;
	case GridBagConstraints::SOUTH:
		r.x += diffx / 2;
		r.y += diffy;
		break;
	case GridBagConstraints::SOUTHWEST:
		r.y += diffy;
		break;
	case GridBagConstraints::WEST:
		r.y += diffy / 2;
		break;
	case GridBagConstraints::NORTHWEST:
		break;
	}
}

/**
 * Figures out the minimum size of the
 * master based on the information from <code>getLayoutInfo</code>.
 * This method should only be used internally by
 * <code>GridBagLayout</code>.
 *
 * @param parent the layout container
 * @param info the layout info for this parent
 * @return a <code>Dimension</code> object containing the
 *   minimum size
 * @since 1.4
 */
void GridBagLayout::getMinSize(Sprite *parent, GridBagLayoutInfo info, float &w,
        float &h, GridInsets &insets) {
    size_t i;
    float t;
    G_UNUSED(parent);

	t = 0;
	for (i = 0; i < info.width; i++)
		t += info.minWidth[i];
    w = t + insets.left + insets.right + ((info.width>1)?((info.width-1)*cellSpacingX):0);

	t = 0;
	for (i = 0; i < info.height; i++)
		t += info.minHeight[i];
    h = t + insets.top + insets.bottom + ((info.height>1)?((info.height-1)*cellSpacingY):0);
}

void GridBagLayout::ArrangeGrid(Sprite *parent,float pwidth,float pheight)  {
	Sprite *comp;
	int compindex;
	GridBagConstraints *constraints;
    GridInsets insets = pInsets;
	Rectangle r;
	float dw, dh;
    size_t i;
	float diffw, diffh;
	double weight;
	GridBagLayoutInfo info;
	std::vector<double> distribute;

	/*
	 * If the parent has no slaves anymore, then don't do anything
	 * at all:  just leave the parent's size as-is.
	 */
	if (parent->childCount() == 0
			&& (columnWidths.size() == 0 || rowHeights.size() == 0)) {
		return;
	}

	/*
	 * Pass #1: scan all the slaves to figure out the total amount
	 * of space needed.
	 */

	info = getLayoutInfo(parent, PREFERREDSIZE);
    getMinSize(parent, info, dw, dh, insets);

    if (resizeContainer) {
    	GridBagLayoutInfo info2;
		float dw2,dh2;
		info2 = getLayoutInfo(parent, MINSIZE);
		getMinSize(parent, info2, dw2, dh2, insets);
		if ((dw2<dw)||(dh2<dh)) {
			info=info2;
			dw=dw2;
			dh=dh2;
		}
		if ((pwidth!=dw)||(pheight!=dh)) {
			pwidth=dw;
			pheight=dh;
			parent->setDimensions(pwidth, pheight);
		}
    }
    else
    {
		if (pwidth < dw || pheight < dh) {
			info = getLayoutInfo(parent, MINSIZE);
			getMinSize(parent, info, dw, dh, insets);
		}
    }

    //Don't try to layout with a too small area
    if (pwidth<dw) pwidth=dw;
    if (pheight<dh) pheight=dh;

	r.width = dw;
	r.height = dh;
	info.reqWidth = dw;
	info.reqHeight = dh;

	/*
	 * If the current dimensions of the window don't match the desired
	 * dimensions, then adjust the minWidth and minHeight arrays
	 * according to the weights.
	 */


    diffw = pwidth - r.width;
	if (diffw != 0) {
		weight = 0.0;
		for (i = 0; i < info.width; i++)
			weight += info.weightX[i];
		if (weight > 0.0) {
			distribute.resize(info.width);
            if (equalizeCells)
                for (i = 0; i < info.width; i++) {
                    if (info.weightX[i]>0) diffw+=info.minWidth[i];
                }
            double perWeight=diffw/weight;
            for (i = 0; i < info.width; i++) {
                float dx;
                if (equalizeCells)
                    dx= (info.weightX[i]>0)?perWeight*info.weightX[i]-info.minWidth[i]:0; //Non weighted cells shouldn't be distributed any space
                else
                    dx= (float) ((((double) diffw) * info.weightX[i]) / weight);
                distribute[i]=dx;
			}
			if (equalizeCells)
			{
				while (true) {
					double neg=0;
					double nweight=0;
                    for (size_t i=0;i<info.width;i++) {
						if (distribute[i]<0)
							neg+=distribute[i];
						else
							nweight+=info.weightX[i];
					}
                    if ((neg>=0)||(nweight<=0)) break;
                    neg/=nweight;
                    for (size_t i=0;i<info.width;i++) {
						if (distribute[i]<0)
							distribute[i]=0;
						else
							distribute[i]+=info.weightX[i]*neg;
					}
				}
			}
            for (i = 0; i < info.width; i++) {
				info.minWidth[i] += distribute[i];
				r.width += distribute[i];
				if (info.minWidth[i] < 0) {
					r.width -= info.minWidth[i];
					info.minWidth[i] = 0;
				}
			}
		}
        diffw = pwidth - r.width;
	}
	else {
		diffw = 0;
	}

    diffh = pheight - r.height;
	if (diffh != 0) {
		weight = 0.0;
		for (i = 0; i < info.height; i++)
			weight += info.weightY[i];
		if (weight > 0.0) {
            if (equalizeCells)
                for (i = 0; i < info.height; i++) {
                    if (info.weightY[i]>0) diffh+=info.minHeight[i];
                }
            double perWeight=diffh/weight;
			distribute.resize(info.height);
            for (size_t i = 0; i < info.height; i++) {
				float dy;
                if (equalizeCells)
                    dy= (info.weightY[i]>0)?perWeight*info.weightY[i]-info.minHeight[i]:0;
                else
                	dy = (float) ((((double) diffh) * info.weightY[i]) / weight);
                distribute[i]=dy;
			}
			if (equalizeCells)
			{
				while (true) {
					double neg=0;
					double nweight=0;
                    for (size_t i=0;i<info.height;i++) {
						if (distribute[i]<0)
							neg+=distribute[i];
						else
							nweight+=info.weightY[i];
					}
                    if ((neg>=0)||(nweight<=0)) break;
					neg/=nweight;
                    for (size_t i=0;i<info.height;i++) {
						if (distribute[i]<0)
							distribute[i]=0;
						else
							distribute[i]+=info.weightY[i]*neg;
					}
				}
			}
			for (i = 0; i < info.height; i++) {
				info.minHeight[i] += distribute[i];
				r.height += distribute[i];
				if (info.minHeight[i] < 0) {
					r.height -= info.minHeight[i];
					info.minHeight[i] = 0;
				}
			}
		}
        diffh = pheight - r.height;
	}
	else {
		diffh = 0;
	}

	/*
	 * Now do the actual layout of the slaves using the layout information
	 * that has been collected.
	 */

	info.startx = diffw / 2 + insets.left;
	info.starty = diffh / 2 + insets.top;

    layoutInfo = info;
    for (compindex = 0; compindex < parent->childCount(); compindex++) {
		comp = parent->child(compindex);
		if ((!comp->visible())||(!comp->layoutConstraints))
			continue;
		constraints = lookupConstraints(comp);

		r.x = info.startx;
		for (i = 0; i < constraints->tempX; i++)
			r.x += info.minWidth[i] + cellSpacingX;

		r.y = info.starty;
		for (i = 0; i < constraints->tempY; i++)
			r.y += info.minHeight[i] + cellSpacingY;

		r.width = 0;
		for (i = constraints->tempX;
				i < (constraints->tempX + constraints->tempWidth); i++) {
			r.width += info.minWidth[i] + cellSpacingX;
		}
		if (constraints->tempWidth>0) r.width-=cellSpacingX;

		r.height = 0;
		for (i = constraints->tempY;
				i < (constraints->tempY + constraints->tempHeight); i++) {
			r.height += info.minHeight[i] + cellSpacingY;
		}
		if (constraints->tempHeight>0) r.height-=cellSpacingY;

		componentAdjusting = comp;
		AdjustForGravity(constraints, r);

		if (r.x < 0) {
			r.width += r.x;
			r.x = 0;
		}

		if (r.y < 0) {
			r.height += r.y;
			r.y = 0;
		}

		//Last step: displace the component according to its origin/offset
		r.x+=constraints->offsetX+constraints->originX*r.width;
		r.y+=constraints->offsetY+constraints->originY*r.height;

        comp->setBounds(r.x, r.y, r.width, r.height,true);
        if (comp->layoutState&&comp->layoutState->dirty)
        {
            comp->layoutState->dirty=false;
            comp->layoutState->ArrangeGrid(comp,r.width,r.height);
        }
	}
}
