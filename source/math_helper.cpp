#include "math_helper.h"

namespace CS350
{

DirectX::XMMATRIX InverseTranspose(DirectX::XMMATRIX const mtx)
{
  return DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(nullptr, mtx));
}

}