function(semver string_or_version)
  if(NOT string_or_version)
    return()
  endif()
  is_map(${string_or_version} )
  ans(ismap)
  if(ismap)
    return(${string_or_version})
  endif()
  semver_parse_lazy(${string_or_version})
  ans(version)
  return(${version})
endfunction()