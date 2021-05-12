#ifndef CIV4_PLOT_REGION_H
#define CIV4_PLOT_REGION_H

#include <map>
#include <set>

#define GET_COORDS(it) (it->first)
#define GET_VISIBILITY(it) (it->second)

inline XYCoords getXYCoords(const std::set<XYCoords>::const_iterator itPlotRegion)
{
	return XYCoords(*itPlotRegion);
}

inline XYCoords getXYCoords(const std::map<XYCoords,int>::const_iterator itPlotRegion)
{
	return XYCoords((itPlotRegion->first));
}

inline XYCoords makeXYCoords(const std::set<XYCoords>::const_iterator itPlotRegion, XYCoords xy)
{
	return xy;
}

inline std::pair<XYCoords,int> makeXYCoords(const std::map<XYCoords,int>::const_iterator itPlotRegion, XYCoords xy)
{
	return std::pair<XYCoords,int>(xy, 0);
}

template <class _RegionIter>
inline int getRegionValue(const _RegionIter itRegion)
{
	return itRegion->second;
}

template <class _RegionIter>
inline void setRegionValue(const _RegionIter itRegion, int iValue)
{
	itRegion->second = iValue;
}

// increases the size of regionToAddTo by adding all plots in regionToAdd 
template <class _RegionClass>
void regionUnion(_RegionClass& regionToAddTo, const _RegionClass& regionToAdd)
{ 
#ifdef check_our_work
	_RegionClass addToCopy = regionToAddTo;
#endif

	// loop both until finished adding (both are in sorted order)
	_RegionClass::iterator itAddTo = regionToAddTo.begin();
	_RegionClass::const_iterator itAdd = regionToAdd.begin();
	while (itAdd != regionToAdd.end())
	{
		bool bAtAddToEnd = (itAddTo == regionToAddTo.end());

		XYCoords addXY = getXYCoords(itAdd);
		XYCoords addToXY;
		if (!bAtAddToEnd)
		{
			addToXY = getXYCoords(itAddTo);
		}
		
		// if not at addTo end and addTo less, increment addTo and loop again
		if (!bAtAddToEnd && addToXY < addXY)
		{
			itAddTo++;
		}
		// if at addTo end or addTo more, insert here, increment add and loop again
		else if (bAtAddToEnd || addXY < addToXY)
		{
			regionToAddTo.insert(itAddTo, (*itAdd));
			itAdd++;
		}
		// otherwise they must be equal, it is in both, increment both
		else
		{
			FAssert(!bAtAddToEnd);
			FAssert(addToXY == addXY);
			itAddTo++;
			itAdd++;
		}
	}

#ifdef check_our_work
	for (_RegionClass::const_iterator it = regionToAdd.begin(); it != regionToAdd.end(); it++)
	{
		addToCopy.insert((*it));
	}

	FAssert(addToCopy.size() == regionToAddTo.size());

	_RegionClass::const_iterator itCopy = addToCopy.begin();
	for (_RegionClass::const_iterator it = regionToAddTo.begin(); it != regionToAddTo.end(); it++)
	{
		FAssert(*itCopy == *it);
		FAssert(itCopy != addToCopy.end());

		itCopy++;
	}
#endif
}

// subtract removes all plots in regionToSubtract from regionToSubtractFrom (non-symetrical difference) 
template <class _RegionClass>
void regionDifference(_RegionClass& regionToSubtractFrom, const _RegionClass& regionToSubtract)
{ 
	for (_RegionClass::const_iterator it = regionToSubtract.begin(); it != regionToSubtract.end(); it++)
	{
		// if we have this plot in the set, remove it
		_RegionClass::iterator itToSubtract = regionToSubtractFrom.find(getXYCoords(it));
		if (itToSubtract != regionToSubtractFrom.end())
		{
			regionToSubtractFrom.erase(itToSubtract);
		}
	}
}

// removes intersection from both regions
template <class _RegionClass>
void removeIntersection (_RegionClass& regionA, _RegionClass& regionB)
{
	bool bAIsSmaller = regionA < regionB;
	_RegionClass& smallerRegion = (bAIsSmaller) ? regionA : regionB;
	_RegionClass& largerRegion = (bAIsSmaller) ? regionB : regionA;

	// loop over the plots, removing as necessary
	_RegionClass::iterator it = smallerRegion.begin();
	while (it != smallerRegion.end())
	{
		// increment before we work with it, so we can remove it if necessary
		_RegionClass::iterator itSmaller = it;
		it++;

		_RegionClass::iterator itLarger = largerRegion.find(getXYCoords(itSmaller));
		if (itLarger != largerRegion.end())
		{
			largerRegion.erase(itLarger);
			smallerRegion.erase(itSmaller);
		}
	}
}

class CvPlotRegion : public std::set<XYCoords>
{
public:	
	// adding builds a union of the two regions
	void operator+= (const CvPlotRegion& regionToAdd)
	{ 
		regionUnion<CvPlotRegion>(*this, regionToAdd);
	}

	// subtract removes all plots in regionToSubtract (non-symetrical difference) 
	void operator-= (const CvPlotRegion& regionToSubtract)
	{ 
		regionDifference<CvPlotRegion>(*this, regionToSubtract);
	}
	
	// copy the left hand operand into new region, then += that with the right hand operand and return the result
	CvPlotRegion operator+ (const CvPlotRegion& regionToAdd) const 
	{
		CvPlotRegion resultRegion(*this);
		resultRegion += regionToAdd;
		return resultRegion;
	}
};

typedef CvPlotRegion::iterator CvPlotRegionIterator;
typedef CvPlotRegion::const_iterator CvPlotRegionConstIterator;

class CvPlotDataRegion : public std::map<XYCoords,int>
{
public:	
	// adding adds the int values at each plot, building a union of the two regions with a sum of the values
	void operator+= (const CvPlotDataRegion& regionToAdd)
	{ 
		for (CvPlotDataRegion::const_iterator it = regionToAdd.begin(); it != regionToAdd.end(); it++)
		{
			(*this)[(*it).first] += (*it).second;
		}
	}
	
	// subtract finds the _positive_ intersection of the regions, 
	// if a point from regionToSubtract is in this region, then subtract the value from the other set 
	// if the result value is not positive, then remove this point from this set
	// note, this is the only place where negative or zero visibility values cause a removal
	// if -= is never called, then it is entirely legal to have zero or negative values
	void operator-= (const CvPlotDataRegion& regionToSubtract)
	{ 
		for (CvPlotDataRegion::const_iterator it = regionToSubtract.begin(); it != regionToSubtract.end(); it++)
		{
			const XYCoords& xy = (*it).first;
			// make sure we have this plot in the set
			CvPlotDataRegion::iterator itFind = this->find(xy);
			if (itFind != this->end())
			{
				// subtract
				int iNewValue = (*itFind).second - (*it).second;
				if (iNewValue > 0)
				{
					(*itFind).second = iNewValue;
				}
				else
				{
					this->erase(itFind);
				}
			}
		}
	}
	
	// copy the left hand operand into new region, then += that with the right hand operand and return the result
	CvPlotDataRegion operator+ (const CvPlotDataRegion& regionToAdd) const 
	{
		CvPlotDataRegion resultRegion(*this);
		resultRegion += regionToAdd;
		return resultRegion;
	}
};

typedef CvPlotDataRegion::iterator CvPlotDataRegionIterator;
typedef CvPlotDataRegion::const_iterator CvPlotDataRegionConstIterator;
typedef std::pair<XYCoords,int> CvVisibilityRegionPair;

#endif