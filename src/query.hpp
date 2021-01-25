//
// Created by Fiodar Navumenka on 8/23/18.
//

#ifndef QUERY_HPP
#define QUERY_HPP

#include <string>
#include <string_view>
#include <vector>

#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/document/value.hpp>
#include <bsoncxx/builder/basic/array.hpp>

namespace msq {

namespace bson_builder = bsoncxx::builder::basic;

using bsoncxx::builder::basic::concatenate;

using document_builder = bsoncxx::builder::basic::document;
using document = bsoncxx::document::value;
using document_view_or_value = bsoncxx::document::view_or_value;

class key;

class query {
private:
    document_builder and_builder;

    std::vector<document> or_builder;

    document compile()
    {
        if (not and_builder.view().empty()) {
            or_builder.emplace_back(and_builder.extract());
        }

        if (or_builder.size() == 1) {
            return or_builder[0];
        }

        auto fields_builder = bson_builder::array{};

        for (const auto& v : or_builder) {
            fields_builder.append(v);
        }

        return bson_builder::make_document(bson_builder::kvp("$or", fields_builder.extract()));
    }

public:
    template<typename ValueT>
    query(const std::string& name, const ValueT& value)
        : and_builder{},
          or_builder{}
    {
        and_builder.append(bson_builder::kvp(name, value));
    }

    template<typename T>
    std::enable_if_t<std::is_same_v<std::decay_t<T>, query>, query&> operator&&(T&& rhs)
    {
        and_builder.append(concatenate(rhs.compile()));

        return *this;
    }

    template<typename T>
    std::enable_if_t<std::is_same_v<std::decay_t<T>, key>, query&> operator&&(T&& rhs)
    {
        and_builder.append(concatenate(query{rhs}));

        return *this;
    }

    template<typename T>
    std::enable_if_t<std::is_same_v<std::decay_t<T>, query>, query&> operator||(T&& rhs)
    {
        or_builder.emplace_back(concatenate(rhs.compile()));

        return *this;
    }

    operator document()
    {
        return compile();
    }

    operator document_view_or_value()
    {
        return compile();
    }
};

}

#endif //QUERY_HPP