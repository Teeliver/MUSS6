#pragma once
#include <vector>
#include <queue>
#include <limits>
#include <stdexcept>
#include <algorithm>
#include <cstdint>
namespace mu::viewer {
// One bounded search per click. Returned directions use the original MU road table.
template<class Attribute>
inline std::vector<unsigned> find_path(const std::vector<Attribute>& attributes,unsigned sx,unsigned sy,unsigned tx,unsigned ty){
 if(attributes.size()!=65536||sx>=256||sy>=256||tx>=256||ty>=256)throw std::runtime_error("Invalid pathfinding terrain or coordinates");
 auto blocked=[&](int x,int y){return x<0||y<0||x>=256||y>=256||(attributes[y*256+x]&12)!=0;};if(blocked(tx,ty)||sx==tx&&sy==ty)return {};
 constexpr int dx[8]={-1,0,1,1,1,0,-1,-1},dy[8]={-1,-1,-1,0,1,1,1,0};
 auto heuristic=[&](int x,int y){int a=std::abs(x-int(tx)),b=std::abs(y-int(ty));return 10*std::max(a,b)+4*std::min(a,b);};
 struct Node{int score,cost,index;bool operator<(const Node& b)const{return score>b.score;}};
 std::vector<int> cost(65536,std::numeric_limits<int>::max());std::vector<std::int8_t> parent(65536,-1);std::priority_queue<Node> open;int start=sy*256+sx,target=ty*256+tx;cost[start]=0;open.push({heuristic(sx,sy),0,start});unsigned expanded=0;
 while(!open.empty()&&expanded++<65536){auto n=open.top();open.pop();if(n.cost!=cost[n.index])continue;if(n.index==target)break;int x=n.index%256,y=n.index/256;
  for(unsigned d=0;d<8;d++){int nx=x+dx[d],ny=y+dy[d];if(blocked(nx,ny))continue;if(dx[d]&&dy[d]&&(blocked(nx,y)||blocked(x,ny)))continue;int index=ny*256+nx,next=n.cost+((dx[d]&&dy[d])?14:10);if(next>=cost[index])continue;cost[index]=next;parent[index]=d;open.push({next+heuristic(nx,ny),next,index});}
 }
 if(parent[target]<0)return {};std::vector<unsigned> path;for(int index=target;index!=start;){int d=parent[index];if(d<0||path.size()>=512)return {};path.push_back(d);index-=dy[d]*256+dx[d];}std::reverse(path.begin(),path.end());return path;
}
}
