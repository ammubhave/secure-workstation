if (get_window_type() == "WINDOW_TYPE_NORMAL") then
  maximize();
  undecorate_window();
  make_always_on_top();
  set_window_workspace(get_workspace_count());
  set_window_above();
  set_on_top();
end

debug_print('---');
debug_print(get_application_name());
debug_print(get_window_type());
debug_print(get_window_name());
debug_print(get_window_geometry());
