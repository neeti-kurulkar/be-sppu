// SPDX-License-Identifier: MIT
pragma solidity ^0.8.0;

contract StudentData {
    struct Student {
        uint id;
        string name;
        uint age;
        string course;
    }

    Student[] public students;
    mapping(uint => bool) private studentExists;

    event StudentAdded(uint id, string name, uint age, string course);

    function addStudent(
        uint _id,
        string calldata _name,
        uint _age,
        string calldata _course
    ) external {
        require(!studentExists[_id], "Student ID already exists.");
        students.push(Student(_id, _name, _age, _course));
        studentExists[_id] = true;
        emit StudentAdded(_id, _name, _age, _course);
    }

    function getStudent(uint index) external view returns (uint, string memory, uint, string memory) {
        require(index < students.length, "Invalid index.");
        Student memory student = students[index];
        return (student.id, student.name, student.age, student.course);
    }

    function getStudentCount() external view returns (uint) {
        return students.length;
    }

    fallback() external payable {
        revert("Fallback called - function does not exist.");
    }

    receive() external payable {
        // Accept ETH silently
    }
}