
namespace Gomoku { namespace WsPlayer
{

template<class Points>
void exclude_exists(const npoints_t& src,Points& dst)
{
	if(src.empty()||dst.empty())return;

	Points res(dst.size());
	res.erase(
	  std::set_difference(dst.begin(),dst.end(),src.begin(),src.end(),res.begin(),less_point_pr()),
	  res.end());
	std::swap(dst,res);
}

} }//namespace Gomoku
