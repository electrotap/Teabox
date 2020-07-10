##
## 
function(list_extract_defined_values __lst defs)
  if(NOT defs)
    message(FATAL_ERROR "no definitions map specified")
  endif()
  map_tryget("${defs}" positionals)
  ans(positionals)
  map_tryget("${defs}" nonpositionals)
  ans(nonpositionals)
  map_new()
  ans(result)
  foreach(def ${nonpositionals} ${positionals})
    list_extract_defined_value(${__lst} "${def}")    
    ans(value)
    map_tryget(${def} variable_name)
    ans(variable_name)
    map_set(${result} "${variable_name}" "${value}")
  endforeach()
  list(APPEND result ${${__lst}})
  return_ref(result)
endfunction()