#ifndef GRIPPER_H_
#define GRIPPER_H_

/**
   Welcome to the gripper class.  This class shall define the methods
   to directly interface with the COM port to control the gripper.
   At this level, the only actions able to be performed are low level.

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

class Gripper {
private:
    int set_gripper(bool);
    int rotate_gripper(int);
};

#endif
