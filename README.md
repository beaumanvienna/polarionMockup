# PolarionMockup

A lightweight C++ REST API server that emulates the Siemens Polarion ALM REST API. Built for offline development and integration testing of clients such as **JarvisAgent**.

All data is held in memory with seed data (go-kart procurement project, 18 requirements, 2 users). Changes persist only for the lifetime of the process.

---

## Build & Run

### Prerequisites

- Linux (tested on Ubuntu)
- GCC with C++20 support
- [Premake5](https://premake.github.io/) on your `PATH`

### Build (Release)

```bash
cd /path/to/polarionMockup
premake5 gmake
make config=release -j$(nproc)
```

### Start

```bash
./bin/Release/mockup
```

The server listens on **http://0.0.0.0:18080**.

### Authentication

All endpoints except `/health` require a Personal Access Token (PAT) via the Bearer scheme:

```
Authorization: Bearer 1234!@#$
```

If the token is missing or invalid, you'll get `401 Unauthorized`.

---

## Endpoints & Example curl Commands

All paths are prefixed with `/polarion/rest/v1`.

### Health Check

```bash
curl -s http://localhost:18080/polarion/rest/v1/health
```

---

### Projects

**List all projects**

```bash
curl -s -H 'Authorization: Bearer 1234!@#$' \
  http://localhost:18080/polarion/rest/v1/projects
```

**Get single project**

```bash
curl -s -H 'Authorization: Bearer 1234!@#$' \
  http://localhost:18080/polarion/rest/v1/projects/GoKartProcurement
```

---

### Work Items

**List work items** (with pagination and optional Lucene query)

```bash
curl -s -H 'Authorization: Bearer 1234!@#$' \
  'http://localhost:18080/polarion/rest/v1/projects/GoKartProcurement/workitems?page%5Bsize%5D=5&page%5Bnumber%5D=1'
```

**List with query filter**

```bash
curl -s -H 'Authorization: Bearer 1234!@#$' \
  'http://localhost:18080/polarion/rest/v1/projects/GoKartProcurement/workitems?query=status%3Aapproved'
```

**List with sparse fieldsets**

```bash
curl -s -H 'Authorization: Bearer 1234!@#$' \
  'http://localhost:18080/polarion/rest/v1/projects/GoKartProcurement/workitems?fields%5Bworkitems%5D=title,status'
```

**Get single work item**

```bash
curl -s -H 'Authorization: Bearer 1234!@#$' \
  http://localhost:18080/polarion/rest/v1/projects/GoKartProcurement/workitems/REQ-003
```

**Create work item**

```bash
curl -s -X POST \
  -H 'Authorization: Bearer 1234!@#$' \
  -H 'Content-Type: application/json' \
  -d '{
    "data": {
      "attributes": {
        "id": "REQ-100",
        "title": "Fire extinguisher mount",
        "type": "requirement",
        "status": "draft",
        "severity": "major",
        "priority": "80.0",
        "description": {
          "type": "text/plain",
          "value": "Each go-kart shall have a fire extinguisher mount."
        }
      }
    }
  }' \
  http://localhost:18080/polarion/rest/v1/projects/GoKartProcurement/workitems
```

**Update work item (PATCH)**

```bash
curl -s -X PATCH \
  -H 'Authorization: Bearer 1234!@#$' \
  -H 'Content-Type: application/json' \
  -d '{
    "data": {
      "attributes": {
        "status": "approved",
        "priority": "95.0"
      }
    }
  }' \
  http://localhost:18080/polarion/rest/v1/projects/GoKartProcurement/workitems/REQ-003
```

**Delete work item**

```bash
curl -s -X DELETE \
  -H 'Authorization: Bearer 1234!@#$' \
  http://localhost:18080/polarion/rest/v1/projects/GoKartProcurement/workitems/REQ-003
```

**Bulk update work items**

```bash
curl -s -X PATCH \
  -H 'Authorization: Bearer 1234!@#$' \
  -H 'Content-Type: application/json' \
  -d '{
    "data": [
      { "id": "REQ-004", "attributes": { "status": "closed" } },
      { "id": "REQ-005", "attributes": { "status": "closed" } }
    ]
  }' \
  http://localhost:18080/polarion/rest/v1/projects/GoKartProcurement/workitems
```

**Bulk delete work items**

```bash
curl -s -X DELETE \
  -H 'Authorization: Bearer 1234!@#$' \
  -H 'Content-Type: application/json' \
  -d '{
    "data": [
      { "id": "REQ-004" },
      { "id": "REQ-005" }
    ]
  }' \
  http://localhost:18080/polarion/rest/v1/projects/GoKartProcurement/workitems
```

---

### Linked Work Items

**List linked work items**

```bash
curl -s -H 'Authorization: Bearer 1234!@#$' \
  http://localhost:18080/polarion/rest/v1/projects/GoKartProcurement/workitems/REQ-003/linkedworkitems
```

**Add a linked work item**

```bash
curl -s -X POST \
  -H 'Authorization: Bearer 1234!@#$' \
  -H 'Content-Type: application/json' \
  -d '{ "data": { "id": "GoKartProcurement/REQ-006" } }' \
  http://localhost:18080/polarion/rest/v1/projects/GoKartProcurement/workitems/REQ-003/linkedworkitems
```

**Remove a linked work item**

```bash
curl -s -X DELETE \
  -H 'Authorization: Bearer 1234!@#$' \
  http://localhost:18080/polarion/rest/v1/projects/GoKartProcurement/workitems/REQ-003/linkedworkitems/GoKartProcurement/REQ-006
```

---

### Comments

**List comments on a work item**

```bash
curl -s -H 'Authorization: Bearer 1234!@#$' \
  http://localhost:18080/polarion/rest/v1/projects/GoKartProcurement/workitems/REQ-003/comments
```

**Add a comment**

```bash
curl -s -X POST \
  -H 'Authorization: Bearer 1234!@#$' \
  -H 'Content-Type: application/json' \
  -d '{
    "data": {
      "attributes": {
        "text": { "type": "text/plain", "value": "Needs review by safety team." },
        "author": "JC"
      }
    }
  }' \
  http://localhost:18080/polarion/rest/v1/projects/GoKartProcurement/workitems/REQ-003/comments
```

**Update a comment**

```bash
curl -s -X PATCH \
  -H 'Authorization: Bearer 1234!@#$' \
  -H 'Content-Type: application/json' \
  -d '{
    "data": {
      "attributes": {
        "text": { "type": "text/plain", "value": "Updated: reviewed and approved." }
      }
    }
  }' \
  http://localhost:18080/polarion/rest/v1/projects/GoKartProcurement/workitems/REQ-003/comments/comment-1
```

**Delete a comment**

```bash
curl -s -X DELETE \
  -H 'Authorization: Bearer 1234!@#$' \
  http://localhost:18080/polarion/rest/v1/projects/GoKartProcurement/workitems/REQ-003/comments/comment-1
```

---

### Attachments

**List attachments on a work item**

```bash
curl -s -H 'Authorization: Bearer 1234!@#$' \
  http://localhost:18080/polarion/rest/v1/projects/GoKartProcurement/workitems/REQ-003/attachments
```

**Add an attachment (metadata only)**

```bash
curl -s -X POST \
  -H 'Authorization: Bearer 1234!@#$' \
  -H 'Content-Type: application/json' \
  -d '{
    "data": {
      "attributes": {
        "fileName": "spec_v2.pdf",
        "title": "Updated spec",
        "contentType": "application/pdf",
        "length": 204800,
        "author": "JC"
      }
    }
  }' \
  http://localhost:18080/polarion/rest/v1/projects/GoKartProcurement/workitems/REQ-003/attachments
```

**Delete an attachment**

```bash
curl -s -X DELETE \
  -H 'Authorization: Bearer 1234!@#$' \
  http://localhost:18080/polarion/rest/v1/projects/GoKartProcurement/workitems/REQ-003/attachments/attachment-1
```

---

### Documents

**List documents in a space**

```bash
curl -s -H 'Authorization: Bearer 1234!@#$' \
  http://localhost:18080/polarion/rest/v1/projects/GoKartProcurement/spaces/Specifications/documents
```

---

### Collections

**List collections**

```bash
curl -s -H 'Authorization: Bearer 1234!@#$' \
  http://localhost:18080/polarion/rest/v1/projects/GoKartProcurement/collections
```

**Create a collection**

```bash
curl -s -X POST \
  -H 'Authorization: Bearer 1234!@#$' \
  -H 'Content-Type: application/json' \
  -d '{
    "data": {
      "attributes": {
        "id": "safety-reqs",
        "name": "Safety Requirements",
        "description": "All safety-related requirements"
      }
    }
  }' \
  http://localhost:18080/polarion/rest/v1/projects/GoKartProcurement/collections
```

**Update a collection**

```bash
curl -s -X PATCH \
  -H 'Authorization: Bearer 1234!@#$' \
  -H 'Content-Type: application/json' \
  -d '{
    "data": {
      "attributes": {
        "description": "Updated description"
      }
    }
  }' \
  http://localhost:18080/polarion/rest/v1/projects/GoKartProcurement/collections/safety-reqs
```

**Delete a collection**

```bash
curl -s -X DELETE \
  -H 'Authorization: Bearer 1234!@#$' \
  http://localhost:18080/polarion/rest/v1/projects/GoKartProcurement/collections/safety-reqs
```

---

## Endpoint Summary

| Method   | Path                                                        | Description                    |
|----------|-------------------------------------------------------------|--------------------------------|
| GET      | `/health`                                                   | Health check (no auth)         |
| GET      | `/projects`                                                 | List projects                  |
| GET      | `/projects/{id}`                                            | Get project                    |
| GET      | `/projects/{id}/workitems`                                  | List/query work items          |
| GET      | `/projects/{id}/workitems/{wiId}`                           | Get work item                  |
| POST     | `/projects/{id}/workitems`                                  | Create work item(s)            |
| PATCH    | `/projects/{id}/workitems/{wiId}`                           | Update work item               |
| DELETE   | `/projects/{id}/workitems/{wiId}`                           | Delete work item               |
| PATCH    | `/projects/{id}/workitems`                                  | Bulk update work items         |
| DELETE   | `/projects/{id}/workitems`                                  | Bulk delete work items         |
| GET      | `/projects/{id}/workitems/{wiId}/linkedworkitems`           | List linked work items         |
| POST     | `/projects/{id}/workitems/{wiId}/linkedworkitems`           | Add link                       |
| DELETE   | `/projects/{id}/workitems/{wiId}/linkedworkitems/{p}/{w}`   | Remove link                    |
| GET      | `/projects/{id}/workitems/{wiId}/comments`                  | List comments                  |
| POST     | `/projects/{id}/workitems/{wiId}/comments`                  | Add comment                    |
| PATCH    | `/projects/{id}/workitems/{wiId}/comments/{cId}`            | Update comment                 |
| DELETE   | `/projects/{id}/workitems/{wiId}/comments/{cId}`            | Delete comment                 |
| GET      | `/projects/{id}/workitems/{wiId}/attachments`               | List attachments               |
| POST     | `/projects/{id}/workitems/{wiId}/attachments`               | Add attachment                 |
| DELETE   | `/projects/{id}/workitems/{wiId}/attachments/{aId}`         | Delete attachment              |
| GET      | `/projects/{id}/spaces/{spaceId}/documents`                 | List documents in space        |
| GET      | `/projects/{id}/collections`                                | List collections               |
| POST     | `/projects/{id}/collections`                                | Create collection              |
| PATCH    | `/projects/{id}/collections/{cId}`                          | Update collection              |
| DELETE   | `/projects/{id}/collections/{cId}`                          | Delete collection              |

---

## Seed Data

The server starts with the following pre-loaded data:

- **Project**: `GoKartProcurement` — "Go-Kart Procurement"
- **Users**: `JC`, `Bob`
- **Work Items**: 18 requirements (`REQ-001` through `REQ-018`) across 4 chapters
  - Chapter 1: General Performance (REQ-001 to REQ-005)
  - Chapter 2: Propulsion & Brakes (REQ-006 to REQ-010)
  - Chapter 3: Seat & Steering (REQ-011 to REQ-014)
  - Chapter 4: Paint & Livery (REQ-015 to REQ-018)
- **Statuses**: draft, open, reviewed, approved (varied across items)
- **Document**: `GoKartProcurement/Specifications/TechnicalSpec`

---

## License

MIT — Copyright (c) 2026 JC Technolabs
