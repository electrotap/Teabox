

  function(tokens_create tokenlist typelist)
    list(LENGTH typelist token_count)
    math(EXPR last_index "${token_count} - 1")

    if(NOT token_count)
      return()
    endif()

    map_new()
    ans(current_nesting)
    map_set("${current_nesting}" nesting_type root)
    set(nesting_stack)
    foreach(i RANGE 0 ${last_index})
      list(GET typelist ${i} type)
      list(GET tokenlist ${i} value)
    
      map_new()
      ans(token)
      map_set("${token}" value "${value}")
      map_set("${token}" type "${type}")


      if("${type}" MATCHES "^((paren_close)|(brace_close)|(bracket_close))$")
        list(GET nesting_stack 0 parent_nesting)
        list(REMOVE_AT nesting_stack 0)
        map_append("${parent_nesting}" tokens ${current_nesting})
        map_set(${current_nesting} end_token ${token})
        set(current_nesting "${parent_nesting}")
      elseif("${type}" STREQUAL "paren_open")
        list(INSERT nesting_stack 0 ${current_nesting})
        map_new()
        ans(current_nesting)
        map_set(${current_nesting} type paren)
        map_set(${current_nesting} begin_token ${token})
      elseif("${type}" STREQUAL "bracket_open")
        list(INSERT nesting_stack 0 ${current_nesting})
        map_new()
        ans(current_nesting)
        map_set(${current_nesting} type bracket)
        map_set(${current_nesting} begin_token ${token})
      elseif("${type}" STREQUAL "brace_open")
        list(INSERT nesting_stack 0 ${current_nesting})
        map_new()
        ans(current_nesting)
        map_set(${current_nesting} type brace)
        map_set(${current_nesting} begin_token ${token})
      elseif(NOT "${type}" MATCHES "^((white_space)|(separation_close)|(separation_open))$")
        map_append("${current_nesting}" tokens "${token}")
      endif()
    endforeach()

    map_tryget("${current_nesting}" tokens)
    ans(tokens)



    return_ref(tokens)
  endfunction()