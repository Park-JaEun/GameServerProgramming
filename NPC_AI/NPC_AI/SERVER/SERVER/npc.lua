myid = 99999;

function set_uid(x)
   myid = x;
end

function event_player_move(player)
   player_x = API_get_x(player);
   player_y = API_get_y(player);
   my_x = API_get_x(myid);
   my_y = API_get_y(myid);
   if (player_x == my_x) then
      if (player_y == my_y) then
         API_SendMessage(myid, player, "HELLO");
      end
   end
  end

function event_player_move_done(player)
   my_move_time = API_get_npc_move_time(myid);
   if(my_move_time == 3) then
      API_SendMessage_BYE(myid, player, "BYE");
   end
 end