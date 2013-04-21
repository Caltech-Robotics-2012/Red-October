/**
   Welcome to the dispatcher class, it accepts commands from the master
   commander and decomposies them into a more low level commands.
   It then sends the messages to the motion commander or the sensor
   commander.

   For example: The master command would generate a command similar to
   Perform validation gate.

   The dispatch would recieve this command and generate the following
   sequence of commands.

   Normalize Submerine Motion
   Righten Sumberine Position
   Generate Path to New Point
   Sequentially Send Each Point to Motion

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
