#ifndef __MILPCPP_MILPCPP_H__
#define __MILPCPP_MILPCPP_H__

#include<milpcpp/aggregate.h>
#include<milpcpp/bounds.h>
#include<milpcpp/indexing.h>
#include<milpcpp/model.h>
#include<milpcpp/param.h>
#include<milpcpp/utils.h>
#include<milpcpp/var.h>

#define MILPCPP_SET(X) \
struct X:public milpcpp::indexing::index<X>   \
{   \
	explicit X(size_t i):milpcpp::indexing::index<X>(i){} \
};   \
milpcpp::indexing::index_set __##X##internal##__;   \
X::_index_set =  &__##X##internal##__

#define MILPCPP_SET_INIT(X, ...) \
struct X:public milpcpp::indexing::index<X>   \
{   \
	explicit X(size_t i):milpcpp::indexing::index<X>(i){} \
};   \
milpcpp::indexing::index_set __##X##internal##__ {__VA_ARGS__ };   \
X::_index_set =  &__##X##internal##__

#define MILPCPP_TYPED_PARAM(X) struct X:public milpcpp::indexing::range_bound<X> { };

#endif
