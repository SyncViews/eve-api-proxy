#pragma once
template<typename T>
struct CrestPage
{
    unsigned page_count;
    unsigned total_count;
    std::vector<T> items;
};

/**Read a response page from EVE CREST.
 * page->items will always be appended to, while page_count and total_count get overwritten,
 * so a single CrestPage object can be used to read all items from multiple requests, allthough the
 * designed of the CREST API means that total_count == items.size() can not be assumed.
 */
template<typename T>
void read_json(json::Parser &parser, CrestPage<T> *page)
{
    static const auto reader = json::ObjectFieldReader<CrestPage<T>, json::IgnoreUnknown>().
        template add<decltype(CrestPage<T>::page_count), &CrestPage<T>::page_count>("pageCount").
        template add<decltype(CrestPage<T>::total_count), &CrestPage<T>::total_count>("totalCount").
        template add<decltype(CrestPage<T>::items), &CrestPage<T>::items>("items");
     reader.read(parser, day);
}

