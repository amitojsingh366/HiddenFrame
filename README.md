This repo contains the team charter document (charter.md), team's initial project proposal (proposal.md), The teams standard's document (standards.md), and project HiddenFrame's requirements document(requirements.md).

Details on the project can be found at url:
csci.viu.ca/~wesselsd/courses/csci265/project.html

- Phase 1 - Complete
- Phase 2 - Complete
- Phase 3 - Complete
- Phase 4 - Complete
- Phase 5 - In-progress
- Phase 6 - Not Started

## How to get strated

### Backend

1. Ensure [boost](https://www.boost.org/) is installed
2. Ensure [gcc suite](https://gcc.gnu.org/) is installed
3. `cd apps/backend`
4. Run `make run` in the `apps/backend` directory (please see [apps/backend/README.md](./apps/backend/README.md) for OS specific instructions)
5. The server should be exposed on port on port `8080` ([`http://localhost:8080`](http://localhost:8080))

### Web

1. Ensure [node.js](https://nodejs.org/en) is installed
2. `cd apps/web`
3. Run `npm i` in `apps/web` directory to install dependencies
4. Run `npm run build` to build a production build of the web app
5. Run `npm start` to serve the web app on port `3000` ([`http://localhost:3000`](http://localhost:3000))
