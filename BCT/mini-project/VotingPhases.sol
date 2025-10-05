// SPDX-License-Identifier: MIT
pragma solidity ^0.8.18;

contract VotingPhases {
    enum State { NotStarted, Voting, Ended }
    State public state;

    address public admin;
    uint public candidatesCount;
    mapping(uint => Candidate) public candidates;
    mapping(address => bool) public voters;

    struct Candidate {
        uint id;
        string name;
        uint voteCount;
    }

    event Voted(address indexed voter, uint candidateId);
    event StateChanged(State newState);
    event CandidateAdded(uint indexed candidateId, string name);
    event ElectionStarted();
    event ElectionEnded();

    constructor(string[] memory candidateNames) {
        admin = msg.sender;
        state = State.NotStarted;
        for (uint i = 0; i < candidateNames.length; i++) {
            candidates[i + 1] = Candidate(i + 1, candidateNames[i], 0);
            candidatesCount++;
            emit CandidateAdded(i + 1, candidateNames[i]);
        }
    }

    modifier onlyAdmin() {
        require(msg.sender == admin, "Only admin can perform this action");
        _;
    }

    modifier inState(State _state) {
        require(state == _state, "Invalid phase for this action");
        _;
    }

    function startVoting() public onlyAdmin inState(State.NotStarted) {
        state = State.Voting;
        emit StateChanged(state);
        emit ElectionStarted();
    }

    function endVoting() public onlyAdmin inState(State.Voting) {
        state = State.Ended;
        emit StateChanged(state);
        emit ElectionEnded();
    }

    function vote(uint candidateId) public inState(State.Voting) {
        require(!voters[msg.sender], "You have already voted");
        require(candidateId > 0 && candidateId <= candidatesCount, "Invalid candidate ID");

        voters[msg.sender] = true;
        candidates[candidateId].voteCount++;
        emit Voted(msg.sender, candidateId);
    }

    function getCandidate(uint id) public view returns (Candidate memory) {
        return candidates[id];
    }

    function getWinner() public view inState(State.Ended) returns (Candidate memory) {
        Candidate memory winner = candidates[1];
        for (uint i = 2; i <= candidatesCount; i++) {
            if (candidates[i].voteCount > winner.voteCount) {
                winner = candidates[i];
            }
        }
        return winner;
    }
}